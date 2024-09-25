//
// Created by fox on 9/25/24.
//

#include <iostream>
#include <vector>
#include <array>
#include <memory>

#include <argparse/argparse.hpp>
#include <podofo/podofo.h>


static const std::array<PoDoFo::PdfColor, 4> colorLibrary = {{
        { 1.0, 0.0, 0.0 }, // Red
        { 229.0/255.0, 126.0/255.0, 24.0/255.0 }, // Yellow
        { 76.0/255.0, 0.0, 101.0/255.0 }, // Purple
        { 0.0, 178.0/255.0, 0.0 }, // Green
}};

// Base-4 conversion
std::vector<PoDoFo::PdfColor> getDotsForNum(uint32_t num) {
    std::vector<PoDoFo::PdfColor> out;

    for(int i = 0; i <= 20; i++) {
        auto col = colorLibrary[num % colorLibrary.size()];
        out.push_back(col);
        num = num / colorLibrary.size();
    }

    return out;
}

int main(int argc, char **argv) {
    argparse::ArgumentParser program("pdf_identify", "0.1.0");

    program.add_argument("path")
           .help("The PDF file to add dot-markers to.");

    program.add_argument("-i", "--starting-id")
           .help("The ID to start numbering the pages at")
           .scan<'i', int>()
           .default_value(0);

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    //////////////////////////////////////////////////////////////////////////
    auto path = program.get("path");
    auto starting_id = program.get<int>("-i");
    std::cout << "Going to add markers to " << path << " starting with " << starting_id << std::endl;

    PoDoFo::PdfMemDocument doc;
    doc.Load(path);

    auto& pages = doc.GetPages();
    auto pageCount = pages.GetCount();

    std::cout << "Document has " << pageCount << " pages...";

    // in pts.
    constexpr double margin = 72.0 / 2;
    constexpr double dotRadius = 72.0 / 4;
    constexpr double dotDistance = 72.0 * 0.75;

    try {
        PoDoFo::PdfPainter painter;
        for (int i = 0; i < pageCount; i++) {
            auto &page = pages.GetPageAt(i);
            auto box = page.GetCropBox();
            std::cout << "Page " << (i + 1) << " has dimensions: ["
                      << box.X << " "
                      << box.Y << " "
                      << box.Width << " "
                      << box.Height << "]"
                      << std::endl;

            painter.SetCanvas(page);

            auto dots = getDotsForNum(i + starting_id);
            // DEBUG!
            dots[i % dots.size()] = { 0.0, 1.0, 1.0 };

            // There is almost certainly a smarter way of doing this, but like...
            // there's only 20 of these things, and I don't really want to think about
            // a generic way.

            ///// Top Right, counterclockwise /////
            painter.GraphicsState.SetFillColor(dots[0]);
            painter.DrawCircle(box.Width - (0 * dotDistance) - margin, box.Height - (2 * dotDistance) - margin, dotRadius, PoDoFo::PdfPathDrawMode::Fill);

            painter.GraphicsState.SetFillColor(dots[1]);
            painter.DrawCircle(box.Width - (0 * dotDistance) - margin, box.Height - (1 * dotDistance) - margin, dotRadius, PoDoFo::PdfPathDrawMode::Fill);

            painter.GraphicsState.SetFillColor(dots[2]);
            painter.DrawCircle(box.Width - (0 * dotDistance) - margin, box.Height - (0 * dotDistance) - margin, dotRadius, PoDoFo::PdfPathDrawMode::Fill);

            painter.GraphicsState.SetFillColor(dots[3]);
            painter.DrawCircle(box.Width - (1 * dotDistance) - margin, box.Height - (0 * dotDistance) - margin, dotRadius, PoDoFo::PdfPathDrawMode::Fill);

            painter.GraphicsState.SetFillColor(dots[4]);
            painter.DrawCircle(box.Width - (2 * dotDistance) - margin, box.Height - (0 * dotDistance) - margin, dotRadius, PoDoFo::PdfPathDrawMode::Fill);

            ///// Top Left, counterclockwise /////
            painter.GraphicsState.SetFillColor(dots[5]);
            painter.DrawCircle((2 * dotDistance) + margin, box.Height - (0 * dotDistance) - margin, dotRadius, PoDoFo::PdfPathDrawMode::Fill);

            painter.GraphicsState.SetFillColor(dots[6]);
            painter.DrawCircle((1 * dotDistance) + margin, box.Height - (0 * dotDistance) - margin, dotRadius, PoDoFo::PdfPathDrawMode::Fill);

            painter.GraphicsState.SetFillColor(dots[7]);
            painter.DrawCircle((0 * dotDistance) + margin, box.Height - (0 * dotDistance) - margin, dotRadius, PoDoFo::PdfPathDrawMode::Fill);

            painter.GraphicsState.SetFillColor(dots[8]);
            painter.DrawCircle((0 * dotDistance) + margin, box.Height - (1 * dotDistance) - margin, dotRadius, PoDoFo::PdfPathDrawMode::Fill);

            painter.GraphicsState.SetFillColor(dots[9]);
            painter.DrawCircle((0 * dotDistance) + margin, box.Height - (2 * dotDistance) - margin, dotRadius, PoDoFo::PdfPathDrawMode::Fill);

            ///// Bottom Left, counterclockwise /////
            painter.GraphicsState.SetFillColor(dots[10]);
            painter.DrawCircle((0 * dotDistance) + margin, (2 * dotDistance) + margin, dotRadius, PoDoFo::PdfPathDrawMode::Fill);

            painter.GraphicsState.SetFillColor(dots[11]);
            painter.DrawCircle((0 * dotDistance) + margin, (1 * dotDistance) + margin, dotRadius, PoDoFo::PdfPathDrawMode::Fill);

            painter.GraphicsState.SetFillColor(dots[12]);
            painter.DrawCircle((0 * dotDistance) + margin, (0 * dotDistance) + margin, dotRadius, PoDoFo::PdfPathDrawMode::Fill);

            painter.GraphicsState.SetFillColor(dots[13]);
            painter.DrawCircle((1 * dotDistance) + margin, (0 * dotDistance) + margin, dotRadius, PoDoFo::PdfPathDrawMode::Fill);

            painter.GraphicsState.SetFillColor(dots[14]);
            painter.DrawCircle((2 * dotDistance) + margin, (0 * dotDistance) + margin, dotRadius, PoDoFo::PdfPathDrawMode::Fill);

            ///// Bottom Right, counterclockwise /////
            painter.GraphicsState.SetFillColor(dots[15]);
            painter.DrawCircle(box.Width - (2 * dotDistance) - margin, (0 * dotDistance) + margin, dotRadius, PoDoFo::PdfPathDrawMode::Fill);

            painter.GraphicsState.SetFillColor(dots[16]);
            painter.DrawCircle(box.Width - (1 * dotDistance) - margin, (0 * dotDistance) + margin, dotRadius, PoDoFo::PdfPathDrawMode::Fill);

            painter.GraphicsState.SetFillColor(dots[17]);
            painter.DrawCircle(box.Width - (0 * dotDistance) - margin, (0 * dotDistance) + margin, dotRadius, PoDoFo::PdfPathDrawMode::Fill);

            painter.GraphicsState.SetFillColor(dots[18]);
            painter.DrawCircle(box.Width - (0 * dotDistance) - margin, (1 * dotDistance) + margin, dotRadius, PoDoFo::PdfPathDrawMode::Fill);

            painter.GraphicsState.SetFillColor(dots[19]);
            painter.DrawCircle(box.Width - (0 * dotDistance) - margin, (2 * dotDistance) + margin, dotRadius, PoDoFo::PdfPathDrawMode::Fill);
        }

        painter.FinishDrawing();

        std::stringstream newPath;
        newPath << path;
        newPath << ".identified.pdf";

        std::cout << "Gonna save..." << std::endl;
        doc.Save(newPath.str());
        std::cout << "Saved!" << std::endl;
    } catch (PoDoFo::PdfError& e) {
        e.PrintErrorMsg();
        return static_cast<int>(e.GetCode());
    }

    return 0;
}