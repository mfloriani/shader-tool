#pragma once

#include "Graph.h"
#include "Node.h"

#include "Editor\ImGui\imnodes.h"
#include "Editor\ImGui\imgui.h"
//#include "Editor\ImGui\imfilebrowser.h"
//#include "Editor\ImGui\ImGuiFileDialog\ImGuiFileDialog.h"
#include "Editor/ImGui/ImGuiFileBrowser/ImGuiFileBrowser.h"

using UiNodeId = int;

const static int INVALID_ID = -1;

enum class UiNodeType
{
    None,
    Add,
    Multiply,
    Draw,
    Sine,
    Time,
    RenderTarget,
    Primitive,
    VertexShader,
    PixelShader
};

struct UiNode
{
    UiNode() : Type(UiNodeType::None), Id(INVALID_ID) {}
    explicit UiNode(UiNodeType type, UiNodeId id) : Type(type), Id(id) {}
    
    UiNodeType Type;
    UiNodeId Id;

    virtual void Render(Graph<Node>& graph) {}
    virtual void Delete(Graph<Node>& graph) {}

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        out << static_cast<int>(Type) << " " << Id;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        int type;
        in >> type >> Id;
        Type = static_cast<UiNodeType>(type);
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const UiNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, UiNode& n)
    {
        return n.Deserialize(in);
    }
};

struct AddNode : UiNode
{
    explicit AddNode(UiNodeType type, UiNodeId id)
        : UiNode(type, id), Left(INVALID_ID), Right(INVALID_ID)
    {
    }

    explicit AddNode(UiNodeType type, UiNodeId id, NodeId lhs, NodeId rhs)
        : UiNode(type, id), Left(lhs), Right(rhs)
    {
    }

    NodeId Left, Right;

    virtual void Delete(Graph<Node>& graph) override
    {
        graph.EraseNode(Left);
        graph.EraseNode(Right);
    }

    virtual void Render(Graph<Node>& graph) override
    {
        const float node_width = 100.f;
        ImNodes::BeginNode(Id);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("ADD");
        ImNodes::EndNodeTitleBar();
        {
            ImNodes::BeginInputAttribute(Left);
            const float label_width = ImGui::CalcTextSize("left").x;
            ImGui::TextUnformatted("left");
            if (graph.GetNumEdgesFromNode(Left) == 0ull)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(node_width - label_width);
                ImGui::DragFloat("##hidelabel", &graph.GetNode(Left).value, 0.01f);
                ImGui::PopItemWidth();
            }
            ImNodes::EndInputAttribute();
        }

        {
            ImNodes::BeginInputAttribute(Right);
            const float label_width = ImGui::CalcTextSize("right").x;
            ImGui::TextUnformatted("right");
            if (graph.GetNumEdgesFromNode(Right) == 0ull)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(node_width - label_width);
                ImGui::DragFloat("##hidelabel", &graph.GetNode(Right).value, 0.01f);
                ImGui::PopItemWidth();
            }
            ImNodes::EndInputAttribute();
        }

        ImGui::Spacing();

        {
            ImNodes::BeginOutputAttribute(Id);
            const float label_width = ImGui::CalcTextSize("result").x;
            ImGui::Indent(node_width - label_width);
            ImGui::TextUnformatted("result");
            ImNodes::EndOutputAttribute();
        }

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << Left << " " << Right;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        in >> Left >> Right;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const AddNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, AddNode& n)
    {
        return n.Deserialize(in);
    }
};

struct MultiplyNode : UiNode
{
    explicit MultiplyNode(UiNodeType type, UiNodeId id)
        : UiNode(type, id), Left(INVALID_ID), Right(INVALID_ID)
    {
    }

    explicit MultiplyNode(UiNodeType type, UiNodeId id, NodeId lhs, NodeId rhs)
        : UiNode(type, id), Left(lhs), Right(rhs)
    {
    }

    NodeId Left, Right;

    virtual void Delete(Graph<Node>& graph) override
    {
        graph.EraseNode(Left);
        graph.EraseNode(Right);
    }

    virtual void Render(Graph<Node>& graph) override
    {
        const float node_width = 100.0f;
    	ImNodes::BeginNode(Id);

    	ImNodes::BeginNodeTitleBar();
    	ImGui::TextUnformatted("MULTIPLY");
    	ImNodes::EndNodeTitleBar();

    	{
    		ImNodes::BeginInputAttribute(Left);
    		const float label_width = ImGui::CalcTextSize("left").x;
    		ImGui::TextUnformatted("left");
    		if (graph.GetNumEdgesFromNode(Left) == 0ull)
    		{
    			ImGui::SameLine();
    			ImGui::PushItemWidth(node_width - label_width);
    			ImGui::DragFloat(
    				"##hidelabel", &graph.GetNode(Left).value, 0.01f);
    			ImGui::PopItemWidth();
    		}
    		ImNodes::EndInputAttribute();
    	}

    	{
    		ImNodes::BeginInputAttribute(Right);
    		const float label_width = ImGui::CalcTextSize("right").x;
    		ImGui::TextUnformatted("right");
    		if (graph.GetNumEdgesFromNode(Right) == 0ull)
    		{
    			ImGui::SameLine();
    			ImGui::PushItemWidth(node_width - label_width);
    			ImGui::DragFloat(
    				"##hidelabel", &graph.GetNode(Right).value, 0.01f);
    			ImGui::PopItemWidth();
    		}
    		ImNodes::EndInputAttribute();
    	}

    	ImGui::Spacing();

    	{
    		ImNodes::BeginOutputAttribute(Id);
    		const float label_width = ImGui::CalcTextSize("result").x;
    		ImGui::Indent(node_width - label_width);
    		ImGui::TextUnformatted("result");
    		ImNodes::EndOutputAttribute();
    	}

    	ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << Left << " " << Right;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        in >> Left >> Right;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const MultiplyNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, MultiplyNode& n)
    {
        return n.Deserialize(in);
    }
};

struct DrawNode : UiNode
{
    explicit DrawNode(UiNodeType type, UiNodeId id)
        : UiNode(type, id), R(INVALID_ID), G(INVALID_ID), B(INVALID_ID), Model(INVALID_ID), VS(INVALID_ID), PS(INVALID_ID)
    {
    }

    explicit DrawNode(UiNodeType type, UiNodeId id, NodeId r, NodeId g, NodeId b, NodeId model, NodeId vs, NodeId ps)
        : UiNode(type, id), R(r), G(g), B(b), Model(model), VS(vs), PS(ps)
    {
    }

    NodeId R, G, B, Model, VS, PS;

    virtual void Delete(Graph<Node>& graph) override
    {
        graph.EraseNode(R);
        graph.EraseNode(G);
        graph.EraseNode(B);
        graph.EraseNode(Model);
        graph.EraseNode(VS);
        graph.EraseNode(PS);
    }

    virtual void Render(Graph<Node>& graph) override
    {
        const float node_width = 100.0f;
        ImNodes::PushColorStyle(ImNodesCol_TitleBar, IM_COL32(11, 109, 191, 255));
        ImNodes::PushColorStyle(ImNodesCol_TitleBarHovered, IM_COL32(45, 126, 194, 255));
        ImNodes::PushColorStyle(ImNodesCol_TitleBarSelected, IM_COL32(81, 148, 204, 255));
        ImNodes::BeginNode(Id);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("DRAW");
        ImNodes::EndNodeTitleBar();

        ImGui::Dummy(ImVec2(node_width, 0.f));
        		
        {
        	ImNodes::BeginInputAttribute(VS);
        	const float label_width = ImGui::CalcTextSize("vs").x;
        	ImGui::TextUnformatted("vs");
        	if (graph.GetNumEdgesFromNode(VS) == 0ull)
        	{
        		ImGui::SameLine();
        		ImGui::PushItemWidth(node_width - label_width);
        		ImGui::PopItemWidth();
        	}
        	ImNodes::EndInputAttribute();
        }

        ImGui::Spacing();

        {
        	ImNodes::BeginInputAttribute(PS);
        	const float label_width = ImGui::CalcTextSize("ps").x;
        	ImGui::TextUnformatted("ps");
        	if (graph.GetNumEdgesFromNode(PS) == 0ull)
        	{
        		ImGui::SameLine();
        		ImGui::PushItemWidth(node_width - label_width);
        		ImGui::PopItemWidth();
        	}
        	ImNodes::EndInputAttribute();
        }

        ImGui::Spacing();

        {
        	ImNodes::BeginInputAttribute(Model);
        	const float label_width = ImGui::CalcTextSize("model").x;
        	ImGui::TextUnformatted("model");
        	if (graph.GetNumEdgesFromNode(Model) == 0ull)
        	{
        		ImGui::SameLine();
        		ImGui::PushItemWidth(node_width - label_width);
        		ImGui::PopItemWidth();
        	}
        	ImNodes::EndInputAttribute();
        }

        ImGui::Spacing();

        {
        	ImNodes::BeginInputAttribute(R);
        	const float label_width = ImGui::CalcTextSize("r").x;
        	ImGui::TextUnformatted("r");
        	if (graph.GetNumEdgesFromNode(R) == 0ull)
        	{
        		ImGui::SameLine();
        		ImGui::PushItemWidth(node_width - label_width);
        		ImGui::DragFloat("##hidelabel", &graph.GetNode(R).value, 0.01f, 0.f, 1.0f);
        		ImGui::PopItemWidth();
        	}
        	ImNodes::EndInputAttribute();
        }

        ImGui::Spacing();

        {
        	ImNodes::BeginInputAttribute(G);
        	const float label_width = ImGui::CalcTextSize("g").x;
        	ImGui::TextUnformatted("g");
        	if (graph.GetNumEdgesFromNode(G) == 0ull)
        	{
        		ImGui::SameLine();
        		ImGui::PushItemWidth(node_width - label_width);
        		ImGui::DragFloat("##hidelabel", &graph.GetNode(G).value, 0.01f, 0.f, 1.f);
        		ImGui::PopItemWidth();
        	}
        	ImNodes::EndInputAttribute();
        }

        ImGui::Spacing();

        {
        	ImNodes::BeginInputAttribute(B);
        	const float label_width = ImGui::CalcTextSize("b").x;
        	ImGui::TextUnformatted("b");
        	if (graph.GetNumEdgesFromNode(B) == 0ull)
        	{
        		ImGui::SameLine();
        		ImGui::PushItemWidth(node_width - label_width);
        		ImGui::DragFloat("##hidelabel", &graph.GetNode(B).value, 0.01f, 0.f, 1.0f);
        		ImGui::PopItemWidth();
        	}
        	ImNodes::EndInputAttribute();
        }

        ImGui::Spacing();

        {
        	ImNodes::BeginOutputAttribute(Id);
        	const float label_width = ImGui::CalcTextSize("output").x;
        	ImGui::Indent(node_width - label_width);
        	ImGui::TextUnformatted("output");
        	ImNodes::EndInputAttribute();
        }

        ImNodes::EndNode();
        ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();
        ImNodes::PopColorStyle();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << R << " " << G << " " << B << " " << Model << " " << VS << " " << PS;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        in >> R >> G >> B >> Model >> VS >> PS;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const DrawNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, DrawNode& n)
    {
        return n.Deserialize(in);
    }
};

struct SineNode : UiNode
{
    explicit SineNode(UiNodeType type, UiNodeId id)
        : UiNode(type, id), Input(INVALID_ID)
    {
    }

    explicit SineNode(UiNodeType type, UiNodeId id, NodeId input)
        : UiNode(type, id), Input(input)
    {
    }

    NodeId Input;

    virtual void Delete(Graph<Node>& graph) override
    {
        graph.EraseNode(Input);
    }

    virtual void Render(Graph<Node>& graph) override
    {
        const float node_width = 100.0f;
        ImNodes::BeginNode(Id);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("SINE");
        ImNodes::EndNodeTitleBar();

        {
        	ImNodes::BeginInputAttribute(Input);
        	const float label_width = ImGui::CalcTextSize("number").x;
        	ImGui::TextUnformatted("number");
        	if (graph.GetNumEdgesFromNode(Input) == 0ull)
        	{
        		ImGui::SameLine();
        		ImGui::PushItemWidth(node_width - label_width);
        		ImGui::DragFloat("##hidelabel", &graph.GetNode(Input).value, 0.01f, 0.f, 1.0f);
        		ImGui::PopItemWidth();
        	}
        	ImNodes::EndInputAttribute();
        }

        ImGui::Spacing();

        {
        	ImNodes::BeginOutputAttribute(Id);
        	const float label_width = ImGui::CalcTextSize("output").x;
        	ImGui::Indent(node_width - label_width);
        	ImGui::TextUnformatted("output");
        	ImNodes::EndInputAttribute();
        }

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << Input;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        in >> Input;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const SineNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, SineNode& n)
    {
        return n.Deserialize(in);
    }
};

struct PrimitiveNode : UiNode
{
    explicit PrimitiveNode(UiNodeType type, UiNodeId id, std::vector<const char*>& primitives)
        : UiNode(type, id), Primitives(primitives), Model(INVALID_ID)
    {
    }

    explicit PrimitiveNode(UiNodeType type, UiNodeId id, std::vector<const char*>& primitives, NodeId model)
        : UiNode(type, id), Primitives(primitives), Model(model)
    {
    }

    std::vector<const char*>& Primitives;
    NodeId Model;

    virtual void Delete(Graph<Node>& graph) override
    {
        graph.EraseNode(Model);
    }

    virtual void Render(Graph<Node>& graph) override
    {
        const float node_width = 100.0f;
        ImNodes::BeginNode(Id);
        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("PRIMITIVE");
        ImNodes::EndNodeTitleBar();

        ImGui::PushItemWidth(node_width);
        auto &modelNode = graph.GetNode(Model);
        int value = static_cast<int>(modelNode.value);
        ImGui::Combo("##hidelabel", &value, Primitives.data(), (int)Primitives.size());
        ImGui::PopItemWidth();
        modelNode.value = static_cast<float>(value);
        
        ImNodes::BeginOutputAttribute(Id);
        const float label_width = ImGui::CalcTextSize("output").x;
        ImGui::Indent(node_width - label_width);
        ImGui::Text("output");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << Model;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        in >> Model;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const PrimitiveNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, PrimitiveNode& n)
    {
        return n.Deserialize(in);
    }
};

struct RenderTargetNode : UiNode
{
    explicit RenderTargetNode(UiNodeType type, UiNodeId id, RenderTexture* renderTexture)
        : UiNode(type, id), RenderTex(renderTexture), Input(INVALID_ID)
    {
    }

    explicit RenderTargetNode(UiNodeType type, UiNodeId id, RenderTexture* renderTexture, NodeId input)
        : UiNode(type, id), RenderTex(renderTexture), Input(input)
    {
    }

    RenderTexture* RenderTex;
    NodeId Input;

    virtual void Delete(Graph<Node>& graph) override
    {
        graph.EraseNode(Input);
    }

    virtual void Render(Graph<Node>& graph) override
    {
        const float node_width = 100.0f;
        ImNodes::BeginNode(Id);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("RENDER TARGET");
        ImNodes::EndNodeTitleBar();

        {
        	ImNodes::BeginInputAttribute(Input);
        	const float label_width = ImGui::CalcTextSize("input").x;
        	ImGui::TextUnformatted("input");
        	if (graph.GetNumEdgesFromNode(Input) == 0ull)
        	{
        		ImGui::SameLine();
        		ImGui::PushItemWidth(node_width - label_width);
        		ImGui::PopItemWidth();
        	}
        	ImNodes::EndInputAttribute();

        	static int w = 256;
        	static int h = 256;
        	//if(_RenderTargetReady)
        		ImGui::Image((ImTextureID)RenderTex->SRV().ptr, ImVec2((float)w, (float)h));
        }

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << Input;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        in >> Input;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const RenderTargetNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, RenderTargetNode& n)
    {
        return n.Deserialize(in);
    }
};

struct TimeNode : UiNode
{
    explicit TimeNode(UiNodeType type, UiNodeId id)
        : UiNode(type, id)
    {
    }

    virtual void Delete(Graph<Node>& graph) override
    {
    }

    virtual void Render(Graph<Node>& graph) override
    {
        ImNodes::BeginNode(Id);

        ImNodes::BeginNodeTitleBar();
        ImGui::TextUnformatted("TIME");
        ImNodes::EndNodeTitleBar();

        ImNodes::BeginOutputAttribute(Id);
        ImGui::Text("output");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const TimeNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, TimeNode& n)
    {
        return n.Deserialize(in);
    }
};

struct ShaderNode : UiNode
{
    explicit ShaderNode(UiNodeType type, UiNodeId id)
        : UiNode(type, id), Shader(INVALID_ID)
    {
        Init();
    }

    explicit ShaderNode(UiNodeType type, UiNodeId id, NodeId shader)
        : UiNode(type, id), Shader(shader)
    {
        Init();
    }

    NodeId Shader;
    //ImGui::FileBrowser FileDialog{ ImGuiFileBrowserFlags_CloseOnEsc };
    imgui_addons::ImGuiFileBrowser file_dialog;

    void Init()
    {
        //FileDialog.SetTitle("Select shader file");
        //FileDialog.SetTypeFilters({ ".hlsl" });
    }

    virtual void Delete(Graph<Node>& graph) override
    {
        graph.EraseNode(Shader);
    }

    virtual void Render(Graph<Node>& graph) override
    {
        ImNodes::BeginNode(Id);

        ImNodes::BeginNodeTitleBar();
        switch (Type)
        {
        case UiNodeType::VertexShader:
            ImGui::TextUnformatted("VERTEX SHADER");
            break;
        case UiNodeType::PixelShader:
            ImGui::TextUnformatted("PIXEL SHADER");
            break;
        default:
            ImGui::TextUnformatted("UNKNOWN SHADER");
            break;
        }
        ImNodes::EndNodeTitleBar();

        if (ImGui::Button("..."))
            //FileDialog.Open();
            //ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", "Choose File", ".cpp,.h,.hpp", ".");
            ImGui::OpenPopup("Open File");
            
        if (file_dialog.showFileDialog("Open File", imgui_addons::ImGuiFileBrowser::DialogMode::OPEN, ImVec2(700, 310), ".rar,.zip,.7z"))
        {
            std::cout << file_dialog.selected_fn << std::endl;      // The name of the selected file or directory in case of Select Directory dialog mode
            std::cout << file_dialog.selected_path << std::endl;    // The absolute path to the selected file
        }

        // display
        //if (ImGuiFileDialog::Instance()->Display("ChooseFileDlgKey"))
        //{
        //    // action if OK
        //    if (ImGuiFileDialog::Instance()->IsOk())
        //    {
        //        std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
        //        std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
        //        // action
        //    }

        //    // close
        //    ImGuiFileDialog::Instance()->Close();
        //}
        
        //FileDialog.Display();
        //if (FileDialog.HasSelected())
        //{
        //    LOG_TRACE("Selected filename {0}", FileDialog.GetSelected().string());
        //    FileDialog.ClearSelected();
        //}

        ImNodes::BeginOutputAttribute(Id);
        ImGui::Text("output");
        ImNodes::EndOutputAttribute();

        ImNodes::EndNode();
    }

    virtual std::ostream& Serialize(std::ostream& out) const
    {
        UiNode::Serialize(out);
        out << " " << Shader;
        return out;
    }

    virtual std::istream& Deserialize(std::istream& in)
    {
        in >> Shader;
        return in;
    }

    friend std::ostream& operator<<(std::ostream& out, const ShaderNode& n)
    {
        return n.Serialize(out);
    }

    friend std::istream& operator>>(std::istream& in, ShaderNode& n)
    {
        return n.Deserialize(in);
    }
};


