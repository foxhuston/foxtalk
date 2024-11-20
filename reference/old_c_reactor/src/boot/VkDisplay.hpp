//
// Created by fox on 9/12/24.
//

#ifndef FOXTALK_TEST_VKDISPLAY_H
#define FOXTALK_TEST_VKDISPLAY_H

#include "CoreRenderer.hpp"
#include <iostream>
#include <cstdint>
#include <optional>
#include <functional>
#include <stdexcept>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>



class VkDisplay : public CoreRenderer {

public:
  std::vector<const char*> extensions() const {
    return std::vector<const char*> {
      VK_KHR_DISPLAY_EXTENSION_NAME
      , VK_KHR_SURFACE_EXTENSION_NAME
    };
  };

  int rankPhysicalDevice(const vk::PhysicalDevice& physicalDevice) const {
    uint32_t display_count = 0;
    vkGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice, &display_count, NULL);

    if(display_count == 0) {
      throw std::runtime_error("No available displays!");
    }

    return 0; // For now, just take the first physical device.
  }

  // This is largely copied from the vkCube example, so it's mostly in C.
  vk::SurfaceKHR createRenderSurface(const vk::Instance& instance, const vk::PhysicalDevice& physicalDevice) {
    uint32_t display_count = 0;
    vkGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice,
                                            &display_count, NULL);
    if (!display_count) {
       throw new std::runtime_error("No available display!");
    }
 
    VkDisplayPropertiesKHR *displays =
       (VkDisplayPropertiesKHR *) malloc(display_count * sizeof(*displays));
    vkGetPhysicalDeviceDisplayPropertiesKHR(physicalDevice,
                                            &display_count,
                                            displays);

    // TODO: VkCube expects the user to specify this on launch.
    //       That might be a better idea, but I think for now
    //       this should work fine...
    display_idx = display_count - 1;
 
    if (display_idx < 0) {
       for (uint32_t i = 0; i < display_count; i++) {
          fprintf(stdout, "display [%i]:\n", i);
          fprintf(stdout, "   name: %s\n", displays[i].displayName);
          fprintf(stdout, "   physical dimensions: %ux%u\n",
                  displays[i].physicalDimensions.width,
                  displays[i].physicalDimensions.height);
          fprintf(stdout, "   physical resolution: %ux%u\n",
                  displays[i].physicalResolution.width,
                  displays[i].physicalResolution.height);
          fprintf(stdout, "   plane reorder: %s\n",
                  displays[i].planeReorderPossible ? "yes" : "no");
          fprintf(stdout, "   persistent content: %s\n",
                  displays[i].persistentContent ? "yes" : "no");
       }
       free(displays);
    } else if (display_idx >= display_count) {
       free(displays);
       throw new std::runtime_error(
           std::format("Invalid display index {0}/{1}", display_idx, display_count));
    }

    /* */
    uint32_t mode_count = 0;
    vkGetDisplayModePropertiesKHR(physicalDevice,
                                 displays[display_idx].display,
                                 &mode_count, NULL);
    if (!mode_count) {
      free(displays);

      throw new std::runtime_error(
          std::format("Not mode available for display {0} ({1})",
              display_idx, displays[display_idx].displayName));
    }

    VkDisplayModePropertiesKHR *modes =
      (VkDisplayModePropertiesKHR *) malloc(mode_count * sizeof(*modes));
    vkGetDisplayModePropertiesKHR(physicalDevice,
                                 displays[display_idx].display,
                                 &mode_count, modes);
    if (display_mode_idx < 0) {
      fprintf(stdout,  "display [%i] (%s) modes:\n",
              display_idx, displays[display_idx].displayName);
      for (uint32_t i = 0; i < mode_count; i++) {
         fprintf(stdout, "mode [%i]:\n", i);
         fprintf(stdout, "   visible region: %ux%u\n",
                 modes[i].parameters.visibleRegion.width,
                 modes[i].parameters.visibleRegion.height);
         fprintf(stdout, "   refresh rate: %u\n",
                 modes[i].parameters.refreshRate);
      }
      free(displays);
      free(modes);

      throw new std::runtime_error("No display mode chosen!");

    } else if (display_mode_idx >= mode_count) {
      free(displays);
      free(modes);

      throw new std::runtime_error(
          std::format("Invalid mode index {0}/{1}",
              display_mode_idx, mode_count));
    }

    /* */
    uint32_t plane_count = 0;
    vkGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice,
                                                &plane_count, NULL);
    if (!plane_count) {
      free(displays);
      free(modes);

      throw std::runtime_error(
          std::format("No plane available for display {0} ({1})",
              display_idx, displays[display_idx].displayName));
    }

    VkDisplayPlanePropertiesKHR *planes =
      (VkDisplayPlanePropertiesKHR *) malloc(plane_count * sizeof(*planes));
    vkGetPhysicalDeviceDisplayPlanePropertiesKHR(physicalDevice,
                                                &plane_count, planes);
    if (display_plane_idx < 0) {
      for (uint32_t i = 0; i < plane_count; i++) {
         fprintf(stdout, "display [%i] (%s) plane [%i]\n",
                 display_idx, displays[display_idx].displayName, i);
         fprintf(stdout, "   current stack index: %u\n",
                 planes[i].currentStackIndex);
         fprintf(stdout, "   displays supported:");
         uint32_t supported_display_count = 0;
         vkGetDisplayPlaneSupportedDisplaysKHR(physicalDevice,
                                               i, &supported_display_count, NULL);
         VkDisplayKHR *supported_displays =
            (VkDisplayKHR *) malloc(supported_display_count * sizeof(*supported_displays));
         vkGetDisplayPlaneSupportedDisplaysKHR(physicalDevice,
                                               i, &supported_display_count, supported_displays);
         for (uint32_t j = 0; j < supported_display_count; j++) {
            for (uint32_t k = 0; k < display_count; k++) {
               if (displays[k].display == supported_displays[j]) {
                  fprintf(stdout, " %u", k);
                  break;
               }
            }
         }
         fprintf(stdout, "\n");

         VkDisplayPlaneCapabilitiesKHR plane_caps;
         vkGetDisplayPlaneCapabilitiesKHR(physicalDevice,
                                          modes[display_mode_idx].displayMode,
                                          i,
                                          &plane_caps);
         fprintf(stdout, "   src pos: %ux%u -> %ux%u\n",
                 plane_caps.minSrcPosition.x,
                 plane_caps.minSrcPosition.y,
                 plane_caps.maxSrcPosition.x,
                 plane_caps.maxSrcPosition.y);
         fprintf(stdout, "   src size: %ux%u -> %ux%u\n",
                 plane_caps.minSrcExtent.width,
                 plane_caps.minSrcExtent.height,
                 plane_caps.maxSrcExtent.width,
                 plane_caps.maxSrcExtent.height);
         fprintf(stdout, "   dst pos: %ux%u -> %ux%u\n",
                 plane_caps.minDstPosition.x,
                 plane_caps.minDstPosition.y,
                 plane_caps.maxDstPosition.x,
                 plane_caps.maxDstPosition.y);
         fprintf(stdout, "   dst size: %ux%u -> %ux%u\n",
                 plane_caps.minDstExtent.width,
                 plane_caps.minDstExtent.height,
                 plane_caps.maxDstExtent.width,
                 plane_caps.maxDstExtent.height);
      }
      free(displays);
      free(modes);
      free(planes);

      throw new std::runtime_error("Need to choose a display plane index");

    } else if (display_plane_idx >= plane_count) {
      free(displays);
      free(modes);
      free(planes);

      std::runtime_error(
          std::format("Invalid plane index {0}/{1}",
              display_plane_idx, plane_count));
    }

    VkDisplayModeCreateInfoKHR display_mode_create_info = {
      .sType = VK_STRUCTURE_TYPE_DISPLAY_MODE_CREATE_INFO_KHR,
      .parameters = modes[display_mode_idx].parameters,
    };

    VkDisplayModeKHR displayMode;

    VkResult result =
      vkCreateDisplayModeKHR(physicalDevice,
                             displays[display_idx].display,
                             &display_mode_create_info,
                             NULL, &displayMode);

    if (result != VK_SUCCESS) {
      free(displays);
      free(modes);
      free(planes);
      throw std::runtime_error( "Unable to create mode");
    }

    VkDisplaySurfaceCreateInfoKHR display_plane_surface_create_info = {
      .sType = VK_STRUCTURE_TYPE_DISPLAY_SURFACE_CREATE_INFO_KHR,
      .displayMode = displayMode,
      .planeIndex = static_cast<uint32_t>(display_plane_idx),
      .transform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
      .alphaMode = VK_DISPLAY_PLANE_ALPHA_OPAQUE_BIT_KHR,
      .imageExtent = modes[display_mode_idx].parameters.visibleRegion,
    };

    VkSurfaceKHR surface;

    result =
      vkCreateDisplayPlaneSurfaceKHR(instance,
                                     &display_plane_surface_create_info,
                                     NULL,
                                     &surface);

    _width = modes[display_mode_idx].parameters.visibleRegion.width;
    _height = modes[display_mode_idx].parameters.visibleRegion.height;

    free(displays);
    free(modes);
    free(planes);

    return vk::SurfaceKHR(surface);
  }

  vk::Extent2D getFramebufferSize() const {
    return vk::Extent2D(_width, _height);
  };

  void setResizeCallback(std::function<void(int, int)> callback) {
    _resizeCallback = callback;
  }

  void mainLoop(std::function<void(void)> appTick) {
    while(1) {
      appTick();
    }
  }

  ~VkDisplay() {
  }

  private:
    std::optional<std::function<void(int, int)>> _resizeCallback;
    uint32_t _width;
    uint32_t _height;

    // From VkCube
    int display_idx = -1;
    int display_mode_idx = 1;
    int display_plane_idx = 0;

    static void framebufferResizeCallback() {
    }
};


#endif //FOXTALK_TEST_VKDISPLAY_H
