#include "dllmain.hpp"
#include "Json.hpp"

const std::string IMAGE_URL = "https://www.hivebackpack.com/api/v2/statistics/CreeperG16/goals";

AmethystContext* context = nullptr;
SafetyHookInline _textPacketHandler;

bool expectingConnection = false;

std::string lastScreen = "";

std::string imgPath = "";
int imgHeight = 0;

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

static void OnRenderUi(ScreenView* screenView, UIRenderContext* uiCtx) {
    std::string currScreen = screenView->visualTree->mRootControlName->mName;

    if (currScreen == "hud_screen") renderImage(uiCtx);

    if (currScreen == lastScreen) return;
    if (currScreen == "debug_screen" || currScreen == "toast_screen") return;

    OnScreenChanged(currScreen, lastScreen);

    lastScreen = currScreen;
}

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

void textPacketHandler(size_t a1, size_t a2, size_t a3, std::shared_ptr<TextPacket>* shared) {
    auto ret = [a1, a2, a3, shared] { return _textPacketHandler.call(a1, a2, a3, shared); };

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

    time_t now = std::time(0);
    std::string url = "https://www.hivebackpack.com/api/v2/statistics/CreeperG16/goals?t=" + std::to_string(now);

    Log::Info("Requesting image from '{}'...", url);
    context->mClientInstance->getImageFromUrl(
        url, [](Bedrock::Http::Status status, const Core::Path& path, uint64_t a3) {
            Log::Info("Got image path {}", path.mPathPart.mUtf8StdString);
            imgPath = path.mPathPart.mUtf8StdString;
        }
    );

    return;
}

void renderImage(UIRenderContext* ctx) {
    if (imgPath == "") return;

    ResourceLocation loc(imgPath);
    loc.mFileSystem = ResourceFileSystem::Raw;

    mce::TexturePtr texturePtr = ctx->getTexture(&loc, true);
    Tessellator& tessellator = ctx->mScreenContext->tessellator;

    tessellator.begin(mce::PrimitiveMode::QuadList, 4);
    float x = 20, y = 20, w = 320.0 / 2, h = 156.0 / 2;

    tessellator.vertexUV(Vec3(x, y, 0), 0, 0);
    tessellator.vertexUV(Vec3(x, y + h, 0), 0, 1);
    tessellator.vertexUV(Vec3(x + w, y + h, 0), 1, 1);
    tessellator.vertexUV(Vec3(x + w, y, 0), 1, 0);

    mce::Mesh mesh = tessellator.end(0, "sillymeshname", 0);
    tessellator.clear();

    mce::MaterialPtr* material = reinterpret_cast<mce::MaterialPtr*>(SlideAddress(0x59BD7E0));
    mesh.renderMesh(*ctx->mScreenContext, *material, texturePtr);
}
