#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <string>
#include <vector>

enum class Priority { LOW, MEDIUM, HIGH };

// Input buffers
static char task_name[128] = "";
static char task_descrp[256] = "";
static int selected_priority = 0; // 0=LOW, 1=MEDIUM, 2=HIGH

class Task {
private:
    std::string name;
    std::string descrp;
    Priority priority;

public:
    Task() : name("Unknown"), descrp("Unknown"), priority(Priority::LOW) {}
    Task(const std::string& _name, const std::string& _descrp, Priority _priority)
        : name(_name), descrp(_descrp), priority(_priority) {}

    const std::string& get_task_name() const { return name; }
    const std::string& get_task_descrp() const { return descrp; }
    Priority get_task_priority() const { return priority; }
};

std::vector <Task> Tasks;

// Helper to draw colored priority badge
void drawPriorityBadge(Priority pr) {
    const char* text = "";
    ImVec4 color;

    switch (pr) {
        case Priority::LOW:    text = "LOW";  color = ImVec4(0.3f, 0.8f, 0.3f, 1.0f); break;
        case Priority::MEDIUM: text = "MED";  color = ImVec4(0.9f, 0.9f, 0.3f, 1.0f); break;
        case Priority::HIGH:   text = "HIGH"; color = ImVec4(0.9f, 0.3f, 0.3f, 1.0f); break;
    }

    ImGui::PushStyleColor(ImGuiCol_Button, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, color);
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, color);
    ImGui::Button(text);
    ImGui::PopStyleColor(3);
}

int main() {
    // manually entering some tasks
    Tasks.emplace_back("Eat", "Eat lasagna", Priority::MEDIUM);
    Tasks.emplace_back("Play", "Play football", Priority::LOW);

// Init GLFW
    if (!glfwInit()) return -1;

    // Setup OpenGL version (3.0+ works fine)
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Task Management App", NULL, NULL);
    if (window == NULL) return -1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); 

    // Load a TTF font
    io.Fonts->AddFontFromFileTTF("fonts/Roboto-Regular.ttf", 20.0f);
    io.Fonts->AddFontFromFileTTF("fonts/Roboto-Bold.ttf", 20.0f);  

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    ImGui::StyleColorsDark(); // optional: reset to dark theme first

    ImGuiStyle& style = ImGui::GetStyle();
    style.FrameRounding = 9.0f;
    style.FramePadding  = ImVec2(9, 4);

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Filtering state
    static int filter_priority = -1; // -1 = All, 0 = Low, 1 = Medium, 2 = High

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiIO& io = ImGui::GetIO();

        // --- TOP NAVIGATION BAR ---
        static bool open_add_task = false;

        // Position the bar 10px from top, full width, 50px tall
        ImGui::SetNextWindowPos(ImVec2(0, 10));
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, 50));

        ImGui::Begin("TopBar", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoSavedSettings);

        // Add Task button
        if (ImGui::Button("+ Add Task")) {
            open_add_task = true;  // open popup
        }

        ImGui::SameLine();
        if (ImGui::Button("All"))    filter_priority = -1;
        ImGui::SameLine();
        if (ImGui::Button("Low"))    filter_priority = 0;
        ImGui::SameLine();
        if (ImGui::Button("Medium")) filter_priority = 1;
        ImGui::SameLine();
        if (ImGui::Button("High"))   filter_priority = 2;

        ImGui::End();


        // Open popup if flag is set
        if (open_add_task) {
            ImGui::OpenPopup("Add Task Popup");
            ImGui::SetNextWindowPos(
                ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f),
                ImGuiCond_Always,
                ImVec2(0.5f, 0.5f) // anchor: center
            );
            open_add_task = false;
        }

        // --- FULLSCREEN TASK VIEW ---
        ImGui::SetNextWindowPos(ImVec2(0, 60)); // below the nav bar
        ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x, io.DisplaySize.y - 60));
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove;
        ImGui::Begin("Task List", nullptr, flags);

        for (const auto& task : Tasks) {
            int pr = (task.get_task_priority() == Priority::LOW) ? 0 :
                    (task.get_task_priority() == Priority::MEDIUM) ? 1 : 2;

            // Apply filter
            if (filter_priority != -1 && pr != filter_priority)
                continue;

            // --- Priority dot ---
            ImU32 color;
            switch (task.get_task_priority()) {
                case Priority::LOW:    color = IM_COL32(0, 200, 0, 255); break;   // green
                case Priority::MEDIUM: color = IM_COL32(255, 200, 0, 255); break; // yellow
                case Priority::HIGH:   color = IM_COL32(255, 60, 60, 255); break; // red
            }

            ImVec2 cursor = ImGui::GetCursorScreenPos();
            float radius = 5.0f;
            ImGui::GetWindowDrawList()->AddCircleFilled(
                ImVec2(cursor.x + radius + 2, cursor.y + ImGui::GetTextLineHeight() * 0.5f),
                radius,
                color
            );

            ImGui::Dummy(ImVec2(radius * 2 + 6, 0));
            ImGui::SameLine();

            // --- Task content ---
            ImGui::BeginGroup();

            // Bold task name
            ImGui::PushFont(io.Fonts->Fonts[1]);  // assumes Fonts[1] is a bold font you loaded
            ImGui::TextUnformatted(task.get_task_name().c_str());
            ImGui::PopFont();

            // Smaller gray description
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(180, 180, 180, 255));
            ImGui::SetWindowFontScale(0.85f);
            ImGui::TextUnformatted(task.get_task_descrp().c_str());
            ImGui::SetWindowFontScale(1.0f);
            ImGui::PopStyleColor();

            ImGui::EndGroup();

            ImGui::Separator(); // line between tasks
        }

        ImGui::End();


        // --- ADD TASK POPUP ---
        if (ImGui::BeginPopupModal("Add Task Popup", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::InputText("Name", task_name, IM_ARRAYSIZE(task_name));
            ImGui::InputText("Description", task_descrp, IM_ARRAYSIZE(task_descrp));
            const char* priorities[] = { "LOW", "MEDIUM", "HIGH" };
            ImGui::Combo("Priority", &selected_priority, priorities, IM_ARRAYSIZE(priorities));

            if (ImGui::Button("Add", ImVec2(120, 0))) {
                Priority pr = (selected_priority == 0) ? Priority::LOW :
                            (selected_priority == 1) ? Priority::MEDIUM : Priority::HIGH;
                Tasks.emplace_back(std::string(task_name), std::string(task_descrp), pr);

                // Reset fields
                task_name[0] = '\0';
                task_descrp[0] = '\0';
                selected_priority = 0;

                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        // --- END UI CODE ---

        // Render
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
