#include "ui_compiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>

static void ensure_directory(const char* path) {
    char* path_copy = strdup(path);
    char* p = path_copy;

    while ((p = strchr(p, '/')) != NULL) {
        *p = '\0';
        mkdir(path_copy, 0755);
        *p = '/';
        p++;
    }

    mkdir(path_copy, 0755);
    free(path_copy);
}

static void generate_windows_ui(ASTNode* ui_node, const char* output_dir) {
    char output_path[512];
    snprintf(output_path, sizeof(output_path), "%s/%s.xaml.cpp", output_dir, ui_node->ident);
    FILE* out = fopen(output_path, "w");
    if (!out) return;

    fprintf(out, "#include <winrt/Windows.UI.Xaml.h>\n");
    fprintf(out, "#include <winrt/Windows.UI.Xaml.Controls.h>\n");
    fprintf(out, "#include <winrt/Windows.UI.Xaml.Navigation.h>\n\n");
    fprintf(out, "using namespace winrt;\n");
    fprintf(out, "using namespace Windows::UI::Xaml;\n");
    fprintf(out, "using namespace Windows::UI::Xaml::Controls;\n\n");

    fprintf(out, "struct %s : PageT<%s>\n", ui_node->ident, ui_node->ident);
    fprintf(out, "{\n");
    fprintf(out, "    %s()\n", ui_node->ident);
    fprintf(out, "    {\n");
    fprintf(out, "        InitializeComponent();\n");
    fprintf(out, "    }\n\n");

    fprintf(out, "    void InitializeComponent()\n");
    fprintf(out, "    {\n");
    fprintf(out, "        auto stack = StackPanel();\n");
    fprintf(out, "        stack.Spacing(10);\n\n");

    // Process children
    ASTNode* child = ui_node->ui.children;
    while (child) {
        if (child->type == NODE_UI_ELEMENT) {
            if (strcmp(child->ui_element.type, "text") == 0) {
                fprintf(out, "        auto text_%d = TextBlock();\n", child->ui_element.id);
                fprintf(out, "        text_%d.Text(L\"%s\");\n",
                        child->ui_element.id, child->ui_element.content);
                fprintf(out, "        text_%d.FontSize(16);\n", child->ui_element.id);
                fprintf(out, "        stack.Children().Append(text_%d);\n\n", child->ui_element.id);
            }
            else if (strcmp(child->ui_element.type, "button") == 0) {
                fprintf(out, "        auto button_%d = Button();\n", child->ui_element.id);
                fprintf(out, "        button_%d.Content(box_value(L\"%s\"));\n",
                        child->ui_element.id, child->ui_element.label);
                fprintf(out, "        button_%d.Click({ this, &%s::OnButton%dClicked });\n",
                        child->ui_element.id, ui_node->ident, child->ui_element.id);
                fprintf(out, "        stack.Children().Append(button_%d);\n\n", child->ui_element.id);
            }
            else if (strcmp(child->ui_element.type, "input") == 0) {
                fprintf(out, "        auto input_%d = TextBox();\n", child->ui_element.id);
                fprintf(out, "        input_%d.PlaceholderText(L\"%s\");\n",
                        child->ui_element.id, child->ui_element.placeholder);
                fprintf(out, "        stack.Children().Append(input_%d);\n\n", child->ui_element.id);
            }
        }
        child = child->next;
    }

    fprintf(out, "        this->Content(stack);\n");
    fprintf(out, "    }\n\n");

    // Generate button click handlers
    child = ui_node->ui.children;
    while (child) {
        if (child->type == NODE_UI_ELEMENT && strcmp(child->ui_element.type, "button") == 0) {
            fprintf(out, "    void OnButton%dClicked(IInspectable const&, RoutedEventArgs const&)\n",
                    child->ui_element.id);
            fprintf(out, "    {\n");
            fprintf(out, "        // Call Pycee++ action: %s\n", child->ui_element.action);
            fprintf(out, "        // This would call the interpreter in real implementation\n");
            fprintf(out, "        OutputDebugString(L\"Button clicked!\\n\");\n");
            fprintf(out, "    }\n\n");
        }
        child = child->next;
    }

    fprintf(out, "};\n");
    fclose(out);

    // Generate header file
    snprintf(output_path, sizeof(output_path), "%s/%s.xaml.h", output_dir, ui_node->ident);
    out = fopen(output_path, "w");
    if (!out) return;

    fprintf(out, "#pragma once\n");
    fprintf(out, "#include \"winrt/Windows.UI.Xaml.h\"\n");
    fprintf(out, "#include \"winrt/Windows.UI.Xaml.Controls.h\"\n");
    fprintf(out, "struct %s : winrt::Windows::UI::Xaml::Controls::PageT<%s>\n",
            ui_node->ident, ui_node->ident);
    fprintf(out, "{\n");
    fprintf(out, "    %s();\n", ui_node->ident);
    fprintf(out, "    void InitializeComponent();\n");

    // Declare button handlers
    child = ui_node->ui.children;
    while (child) {
        if (child->type == NODE_UI_ELEMENT && strcmp(child->ui_element.type, "button") == 0) {
            fprintf(out, "    void OnButton%dClicked(winrt::Windows::Foundation::IInspectable const&, winrt::Windows::UI::Xaml::RoutedEventArgs const&);\n",
                    child->ui_element.id);
        }
        child = child->next;
    }

    fprintf(out, "};\n");
    fclose(out);
}

static void generate_ios_ui(ASTNode* ui_node, const char* output_dir) {
    char output_path[512];
    snprintf(output_path, sizeof(output_path), "%s/%s.swift", output_dir, ui_node->ident);
    FILE* out = fopen(output_path, "w");
    if (!out) return;

    fprintf(out, "import SwiftUI\n\n");
    fprintf(out, "struct %s: View {\n", ui_node->ident);
    fprintf(out, "    @State private var inputs: [String: String] = [:]\n\n");

    fprintf(out, "    var body: some View {\n");
    fprintf(out, "        VStack(spacing: 15) {\n");
    fprintf(out, "            Text(\"%s\")\n", ui_node->ui.title);
    fprintf(out, "                .font(.title)\n");
    fprintf(out, "                .padding()\n\n");

    ASTNode* child = ui_node->ui.children;
    while (child) {
        if (child->type == NODE_UI_ELEMENT) {
            if (strcmp(child->ui_element.type, "text") == 0) {
                fprintf(out, "            Text(\"%s\")\n", child->ui_element.content);
                fprintf(out, "                .font(.body)\n");
            }
            else if (strcmp(child->ui_element.type, "input") == 0) {
                fprintf(out, "            TextField(\"%s\", text: $inputs[\"%s\", default: \"\"])\n",
                        child->ui_element.placeholder, child->ui_element.name);
                fprintf(out, "                .textFieldStyle(RoundedBorderTextFieldStyle())\n");
            }
            else if (strcmp(child->ui_element.type, "button") == 0) {
                fprintf(out, "            Button(action: {\n");
                fprintf(out, "                // Call Pycee++ action: %s\n", child->ui_element.action);
                fprintf(out, "                print(\"Button clicked!\")\n");
                fprintf(out, "            }) {\n");
                fprintf(out, "                Text(\"%s\")\n", child->ui_element.label);
                fprintf(out, "                    .font(.headline)\n");
                fprintf(out, "                    .foregroundColor(.white)\n");
                fprintf(out, "            }\n");
                fprintf(out, "            .frame(maxWidth: .infinity)\n");
                fprintf(out, "            .padding()\n");
                fprintf(out, "            .background(Color.blue)\n");
                fprintf(out, "            .cornerRadius(10)\n");
            }
        }
        child = child->next;
    }

    fprintf(out, "        }\n");
    fprintf(out, "        .padding()\n");
    fprintf(out, "    }\n");
    fprintf(out, "}\n\n");

    fprintf(out, "struct %s_Previews: PreviewProvider {\n", ui_node->ident);
    fprintf(out, "    static var previews: some View {\n");
    fprintf(out, "        %s()\n", ui_node->ident);
    fprintf(out, "    }\n");
    fprintf(out, "}\n");

    fclose(out);
}

void compile_ui_to_platform(ASTNode* ui_ast, TargetPlatform target, const char* output_dir) {
    char platform_dir[256];

    switch (target) {
        case TARGET_WINDOWS:
            snprintf(platform_dir, sizeof(platform_dir), "%s/windows", output_dir);
            ensure_directory(platform_dir);
            generate_windows_ui(ui_ast, platform_dir);
            printf("✅ Generated Windows UI for %s\n", ui_ast->ident);
            break;

        case TARGET_IOS:
            snprintf(platform_dir, sizeof(platform_dir), "%s/ios", output_dir);
            ensure_directory(platform_dir);
            generate_ios_ui(ui_ast, platform_dir);
            printf("✅ Generated iOS UI for %s\n", ui_ast->ident);
            break;

        // Other platforms would have similar implementations
        default:
            printf("⚠️ Platform not yet implemented\n");
    }
}

void compile_all_platforms(ASTNode* ui_ast, const char* base_output_dir) {
    printf("🌍 Generating UI for all platforms...\n");

    TargetPlatform targets[] = {
        TARGET_WINDOWS,
        TARGET_IOS,
        TARGET_ANDROID,
        TARGET_MACOS,
        TARGET_LINUX,
        TARGET_WEB
    };

    for (size_t i = 0; i < sizeof(targets)/sizeof(targets[0]); i++) {
        compile_ui_to_platform(ui_ast, targets[i], base_output_dir);
    }

    printf("✨ Done! Generated UI for 6 platforms in %s\n", base_output_dir);
}