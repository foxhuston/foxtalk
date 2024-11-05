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
    { 0.0, 178.0/255.0, 0.0 }, // Green
    { 76.0/255.0, 0.0, 101.0/255.0 }, // Purple
}};

uint32_t manchester(uint16_t n) {
    uint32_t out = 0;
    for(int i = 0; i < 16; i++) {
        auto bit = n % 16;
        auto mc = (bit ^ 0) + ((bit ^ 1) << 1);
        out = (out << 2) + mc;
    }

    return out;
}

// Base-4 conversion
std::vector<PoDoFo::PdfColor> getDotsForNum(uint64_t num) {
    auto n = num;
    std::vector<PoDoFo::PdfColor> out;
    std::deque<uint8_t> debugDigits;

    // 12131133
    for(int i = 0; i < 20; i++) {
        auto digit = num % colorLibrary.size();
        debugDigits.push_front(static_cast<uint8_t>(digit));

        auto col = colorLibrary[digit];
        out.push_back(col);
        num = num / colorLibrary.size();
    }

    std::cout << n << " --base 4--> ";
    int digindex = 0;
    for(auto digit : debugDigits) {
        if((digindex++) % 5 == 0) {
            std::cout << " ";
        }
        std::cout << (uint32_t)digit;
    }
    std::cout << std::endl;

    return out;
}

// 00000 00000 00101 12111
// OORGP POGRO GOPOR RPRGG

// 00000 00000 00101 12112
// OGOPR PGGRO PRGOO RPRPG

// dec 3656
// 00000 00000 00003 21020
// GROGP RGOPP RGOPR OPGRO

int main(int argc, char **argv) {
    std::string path;
    int startingId;
    double dotRadius, margin, dotDistance;

    argparse::ArgumentParser program("pdf_identify", "0.1.0");

    program.add_argument("path")
           .help("The PDF file to add dot-markers to.")
           .store_into(path);

    program.add_argument("-i", "--starting-id")
           .help("The ID to start numbering the pages at")
           .default_value(0)
           .store_into(startingId);

    program.add_argument("-r", "--dot-radius")
           .help("The radius of the identifier dots in pts.")
           .default_value(72.0 * 0.25)
           .store_into(dotRadius);

    program.add_argument("-m", "--dot-margin")
           .help("The margin from the edge of the page to the center of the dots in pts.")
           .default_value(72.0 * 0.5)
           .store_into(margin);

    program.add_argument("-s", "--dot-spacing")
           .help("The space between dot centers (per triangle) in pts.")
           .default_value(72.0 * 0.75)
           .store_into(dotDistance);

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::exception& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    //////////////////////////////////////////////////////////////////////////
    std::cout << "Going to add markers to " << path << " starting with " << startingId << std::endl;

    PoDoFo::PdfMemDocument doc;
    doc.Load(path);

    auto& pages = doc.GetPages();
    auto pageCount = pages.GetCount();

    std::cout << "Document has " << pageCount << " pages...";

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

            auto dotNum = i + startingId;
            // dotNum = manchester(dotNum);
            // Gray code, a-la wikipedia
            // dotNum = dotNum ^ (dotNum >> 1);

            auto dots = getDotsForNum(dotNum);
            // DEBUG!
            // dots[i % dots.size()] = { 0.0, 1.0, 1.0 };

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