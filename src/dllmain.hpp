#pragma once
#include <Windows.h>
#include <amethyst/runtime/AmethystContext.hpp>
#include <minecraft/src/common/network/packet/CommandRequestPacket.hpp>
#include <minecraft/src/common/network/PacketSender.hpp>
#include <minecraft/src/common/network/IPacketHandlerDispatcher.hpp>
#include <minecraft/src/common/network/PacketHandlerDispatcherInstance.hpp>
#include <minecraft/src/common/world/level/BlockSource.hpp>
#include <minecraft/src-client/common/client/renderer/Tessellator.hpp>
#include <minecraft/src-client/common/client/renderer/screen/MinecraftUIRenderContext.hpp>
#include <minecraft/src-client/common/client/renderer/screen/ScreenContext.hpp>
#include <minecraft/src-deps/minecraftrenderer/renderer/Mesh.hpp>
#include <minecraft/src-deps/minecraftrenderer/renderer/MaterialPtr.hpp>
#include <minecraft/src-client/common/client/gui/gui/VisualTree.hpp>
#include <minecraft/src-deps/core/file/Path.hpp>
#include <minecraft/src-client/common/client/renderer/TexturePtr.hpp>
#include <minecraft/src-deps/core/resource/ResourceHelper.hpp>
#include <minecraft/src-client/common/client/gui/ScreenView.hpp>
#include <minecraft/src-client/common/client/gui/gui/UIControl.hpp>
#include <minecraft/src-deps/minecraftrenderer/renderer/BedrockTexture.hpp>
#include <regex>

namespace Bedrock::Http {
    class Status {
    private:
        uint32_t        mValue;
        std::error_code mError;
    };
}

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

#define ModFunction extern "C" __declspec(dllexport)

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}

void OnStartJoinGame(ClientInstance* client);
void OnRenderUi(ScreenView* screenView, UIRenderContext* uiCtx);
void OnScreenChanged(std::string newScreen, std::string oldScreen);

void textPacketHandler(size_t a1, size_t a2, size_t a3, std::shared_ptr<TextPacket>* shared);
void renderImage(UIRenderContext* ctx);
