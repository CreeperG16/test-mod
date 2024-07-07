#include "dllmain.hpp"
#include <minecraft/src/common/network/packet/CommandRequestPacket.hpp>
#include <minecraft/src/common/network/PacketSender.hpp>
#include <minecraft/src/common/network/IPacketHandlerDispatcher.hpp>
#include <minecraft/src/common/network/PacketHandlerDispatcherInstance.hpp>

#include <regex>

class TextPacket : Packet {
public:
    uint8_t mType;
    std::string mAuthor;
    std::string mMessage;
    std::string mFilteredMessage;
    std::vector<std::string> mParams;
    bool mLocalize;
    std::string mXuid;
	std::string mPlatformId;

public:
    virtual MinecraftPacketIds getId() const override;
	virtual std::string getName() const override;
    virtual void write(BinaryStream& stream) override;
	virtual Bedrock::Result<void> _read(ReadOnlyBinaryStream& stream) override;
};

AmethystContext* context = nullptr;

bool expectingConnection = false;

const char* IMAGE_CMD = "curl https://www.hivebackpack.com/api/v2/statistics/CreeperG16/goals -O test.png";
const std::string IMAGE_URL = "https://www.hivebackpack.com/api/v2/statistics/CreeperG16/goals";

static void OnScreenChanged(std::string newScreen, std::string oldScreen) {
	//Log::Info("Screen changed from {0} to {1}", oldScreen, newScreen);

    if (oldScreen != "overworld_loading_progress_screen") return;
    //Log::Info("Requesting connection...");

    CommandRequestPacket packet{};

    packet.mCommand = "/connection";
	packet.mOrigin.mType = 0;
	
    expectingConnection = true;
    context->mClientInstance->getRegion()->mLevel->mPacketSender->sendToServer(packet);
}

std::string imgPath = "";
void renderImage(UIRenderContext* ctx) {
    if (imgPath == "") return;

    //ResourceLocation loc(imgPath);
    //loc.mFileSystem = ResourceFileSystem::Raw;

    ResourceLocation loc("textures/items/apple");
    //loc.mFileSystem = ResourceFileSystem::RawPersistent;

    mce::TexturePtr texturePtr = ctx->getTexture(&loc, true);

    //glm::highp_vec2 pos = { 20, 20 };
	//glm::highp_vec2 size = { 320, 156 };
    //glm::highp_vec2 uv = { 0, 0 };
	//glm::highp_vec2 uvSize = { 1, 1 };
	//ctx->drawImage(texturePtr, &pos, &size, &uv, &uvSize, 1);

    //HashedString flushString(0xA99285D21E94FC80, "ui_flush");
    //ctx->flushImages(mce::Color::RED, 1, flushString);

	Tessellator tessellator = ctx->mScreenContext->tessellator;

    tessellator.begin(mce::PrimitiveMode::QuadList, 4);
    //tessellator.vertexUV(0, 0, 0, 0, 0);
    //tessellator.vertexUV(0, 1, 0, 0, 1);
    //tessellator.vertexUV(1, 1, 0, 1, 1);
    //tessellator.vertexUV(1, 0, 0, 1, 0);

    tessellator.vertexUV(Vec3(0,   0,   0), 0, 0);
    tessellator.vertexUV(Vec3(0,   200, 0), 0, 1);
    tessellator.vertexUV(Vec3(200, 200, 0), 1, 1);
    tessellator.vertexUV(Vec3(200, 0,   0), 1, 0);

    mce::Mesh mesh = tessellator.end(0, "sillymeshname", 0);
    tessellator.clear();

    mce::MaterialPtr* material = reinterpret_cast<mce::MaterialPtr*>(SlideAddress(0x59BD7E0));

    const std::variant<std::monostate, mce::TexturePtr, mce::ClientTexture, mce::ServerTexture> newTexture = texturePtr;

    mesh.renderMesh(*ctx->mScreenContext, *material, newTexture);

    //HashedString flushString(0xA99285D21E94FC80, "ui_flush");
    //ctx->flushImages(mce::Color::RED, 1, flushString);
}

std::string lastScreen = "";
static void OnRenderUi(ScreenView* screenView, UIRenderContext* uiCtx) {
	std::string currScreen = screenView->visualTree->mRootControlName->mName;

    if (currScreen == "hud_screen") renderImage(uiCtx);

    if (currScreen == lastScreen) return;
    if (currScreen == "debug_screen" || currScreen == "toast_screen") return;

    //Log::Info("Screen changed to: {}", currScreen);
	OnScreenChanged(currScreen, lastScreen);

    lastScreen = currScreen;
}

SafetyHookInline _textPacketHandler;

void textPacketHandler(size_t a1, size_t a2, size_t a3, std::shared_ptr<TextPacket>* shared) {
    auto ret = [a1, a2, a3, shared]{ return _textPacketHandler.call(a1, a2, a3, shared); };

    TextPacket* packet = shared->get();
    //Log::Info("Got text: {0} (type = {1}, expecting conn = {2})", packet->mMessage, packet->mType, expectingConnection ? "true" : "false");

    if (!expectingConnection) return ret();
    if (packet->mType != 0) return ret();

    if (std::regex_match(packet->mMessage, std::regex("^You are connected to"))) return ret();

    if (std::regex_match(packet->mMessage, std::regex("^You are connected to server (\\d\\.){4}:\\d+$"))) {
        expectingConnection = false;
        return;
    }

    std::smatch match;
    bool res = std::regex_search(packet->mMessage, match, std::regex("server name ([A-Z\\-]+)\\d+$"));

    if (!res) return;
    std::string server = match[1];

    Log::Info("Joined server '{}'", server);
    
    //Log::Info("Requesting image...");
	context->mClientInstance->getImageFromUrl(
        "https://www.hivebackpack.com/api/v2/statistics/CreeperG16/goals",
        [](Bedrock::Http::Status status, const Core::Path& path, uint64_t a3) {
		    Log::Info("Got image path {}", path.mPathPart.mUtf8StdString);
			imgPath = path.mPathPart.mUtf8StdString;
	    }
    );

    return;
}

// Ran when the mod is loaded into the game by AmethystRuntime
ModFunction void Initialize(AmethystContext* ctx) {
    context = ctx;
    //Log::Info("Hello, Amethyst World! This is another sentence to prove I totally know what I'm doing! (I don't)");
    Log::Info("Init");

    // Add a listener to a built in amethyst event
    ctx->mEventManager.onStartJoinGame.AddListener(&OnStartJoinGame);
    ctx->mEventManager.afterRenderUI.AddListener(&OnRenderUi);

    ctx->mHookManager.RegisterFunction<&textPacketHandler>("49 8B 00 49 8B C8 4D 8B 01 48 8B 80 ? ? ? ? 48 FF 25 ? ? ? ? CC CC CC CC CC CC CC CC CC 49 8B 00 49 8B C8 4D 8B 01 48 8B 80 ? ? ? ? 48 FF 25 ? ? ? ? CC CC CC CC CC CC CC CC CC 48 89 5C 24");
	ctx->mHookManager.CreateHook<&textPacketHandler>(_textPacketHandler, &textPacketHandler);
}

// Subscribed to amethysts on start join game event in Initialize
void OnStartJoinGame(ClientInstance* client) {
    Log::Info("The player has joined the game!");
}
