
#let query(m) = {  smallcaps(text(fill: blue)[#sym.arrow.r #m])}
#let claim(m) = {smallcaps(text(fill: red)[#m #sym.arrow.r])}

#let outvar() = { text[\_\_\_\_] } 

#set document(
  title: [Vulkan Handlers],
  author: ("lexi Huston")
)
// #set page("us-letter", margin: ( x: 0.75in ))
#set page("us-letter")
#set text(size: 10pt)
// #set heading(numbering: "1.1")
// #set math.equation(numbering: "(1)")
#set enum(numbering: "1.a.")

#context text(24pt)[
  *#document.title*
]

#context for a in document.author [
  #a #h(2cm)
]

#v(12pt)

#datetime.today().display()

#v(12pt)


= Vulkan Handler Dependencies

This will explain the vulkan process that are in the handlers, what each handler does, and the things it depends on. This will be useful both for anyone not me (lexi) who wants to understand the code, and for me to optimize what exactly each handler needs to query on and when.

At the very top level, Vulkan has these parts:

A chosen physical device to render images with
A surface to render to
A pipeline to define how to render images
A render loop

All of the handlers described in this document will, at the end of the day, service one of these 4 elements.  

= Vulkan Instance Handler

  Creates the vulkan instance. Top level handler, relying on nothing else-- though does optionally depend on some configuration options:

  #query[vulkan should be running ...]

  #query[? vulkan should have validation layers]


  If this instance ever reloads, it will be a full vulkan pipeline and surface re-creation. We shouldn't really ever need to do this in the normal use of Foxtalk.

  
  #claim[#outvar() is the vulkan instance]
  

= Wayland Instance Handler

  Creates the wayland instance. This is necessary when the surface we are rendering to is a wayland surface:

  
  #query[vulkan should be running on wayland]
  

  This instance handler is responsible for registering callback functions with whatever wayland server is running and then creating (and destroying) wayland surfaces. The types of objects this handler works on uses the `wayland-client` library, and puts a `wl_display` and a `wl_surface` Cptr in the db.

  Whenever the surface needs recreation-- for example, when the size of the window changes-- this is the handler responsible for recreating that surface. 

  #claim[
          #outvar()
          is a
          wayland display
          with wl_surface
          #outvar()
          with width
          #outvar()
          and height
          #outvar()
  ]

== Vulkan Wayland Surface Handler

This handler is responsible for turning the `wl_surface` created by the `Wayland Instance Handler` into something Vulkan can use, a `VkSurfaceKHR`