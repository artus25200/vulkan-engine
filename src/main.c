#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef OSX
#define GLFW_INCLUDE_VULKAN
#else
#include <vulkan/vulkan.h>
#endif
#include <GLFW/glfw3.h>
#define WINDOW_WIDTH 1080
#define WINDOW_HEIGHT 920

#define INFO(fmt, ...) output_log(INFO, fmt, ##__VA_ARGS__)
#define DEBUG(fmt, ...) output_log(DEBUG, fmt, ##__VA_ARGS__)
#define WARN(fmt, ...) output_log(WARN, fmt, ##__VA_ARGS__)
#define ERROR(fmt, ...) output_log(ERROR, fmt, ##__VA_ARGS__)
#define FATAL(fmt, ...) output_log(FATAL, fmt, ##__VA_ARGS__)

enum log_level { INFO, DEBUG, WARN, ERROR, FATAL };

void output_log(int loglevel, const char *fmt, ...) {
  const char *log_levels[5] = {"[INFO]", "[DEBUG]", "[WARN]", "[ERROR]",
                               "[FATAL]"};
  FILE *output[5] = {stdout, stdout, stdout, stderr, stderr};
  fprintf(output[loglevel], "%s ", log_levels[loglevel]);
  va_list va_args;
  va_start(va_args, fmt);
  vfprintf(output[loglevel], fmt, va_args);
  fprintf(output[loglevel], "\n");
  va_end(va_args);
  if (loglevel == FATAL)
    exit(EXIT_FAILURE);
}

void get_best_physical_device(VkPhysicalDevice *bestDevice,
                              VkPhysicalDevice *devices, uint32_t count) {
  int bestScore = 0;
  uint32_t bestDeviceIndex = -1;
  VkPhysicalDeviceProperties properties;
  VkPhysicalDeviceFeatures features;
  for (int i = 0; i < count; i++) {
    int score = 0;
    vkGetPhysicalDeviceProperties(devices[i], &properties);
    vkGetPhysicalDeviceFeatures(devices[i], &features);
    if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
      score += 1000;
    score += properties.limits.maxImageDimension2D;
    if (score > bestScore) {
      bestScore = score;
      bestDeviceIndex = i;
    }
  }
  if (bestDeviceIndex != -1)
    *bestDevice = devices[bestDeviceIndex];
}

struct QueueFamilyIndices {
  uint32_t graphicsFamily;
};

struct QueueFamilyIndices findQueueFamily(VkPhysicalDevice device) {
  struct QueueFamilyIndices indices;
  uint32_t queueFamilyCount = 0;
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, NULL);
  VkQueueFamilyProperties *queueFamilies =
      malloc(sizeof(VkQueueFamilyProperties) * queueFamilyCount);
  vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount,
                                           queueFamilies);
  for (int i = 0; i < queueFamilyCount; ++i) {
    if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
      indices.graphicsFamily = i;
    }
  }
  return indices;
}

int main(int argc, char **argv) {
  INFO("Loading vulkan_engine...");

  uint32_t version = VK_MAKE_VERSION(1, 0, 0);

  VkApplicationInfo application_info = {VK_STRUCTURE_TYPE_APPLICATION_INFO,
                                        NULL,
                                        "Vulkan Engine by Artus25200",
                                        version,
                                        "Custom",
                                        version,
                                        VK_API_VERSION_1_3};

  VkInstanceCreateInfo instance_create_info = {
      VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
      NULL,
      0,
      &application_info,
      0,
      NULL,
      0,
      NULL};
  instance_create_info.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

  uint32_t glfwExtensionCount = 0;
  const char **glfwExtensions;

  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

  char **Extensions = malloc(sizeof(char *) * (glfwExtensionCount));

  memcpy(Extensions, glfwExtensions, sizeof(char *) * glfwExtensionCount);
  instance_create_info.enabledExtensionCount = glfwExtensionCount;
  instance_create_info.ppEnabledExtensionNames = (const char **)Extensions;

  VkInstance instance;
  int errorCode;
  if ((errorCode = vkCreateInstance(&instance_create_info, NULL, &instance)) !=
      VK_SUCCESS) {
    FATAL("Could not create VkInstance, error code : %d", errorCode);
  }
  uint32_t deviceCount = 0;
  vkEnumeratePhysicalDevices(instance, &deviceCount, NULL);

  if (deviceCount == 0) {
    FATAL("No GPUs with Vulkan support");
  }

  VkPhysicalDevice *devices = malloc(sizeof(VkPhysicalDevice) * deviceCount);
  vkEnumeratePhysicalDevices(instance, &deviceCount, devices);

  VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

  get_best_physical_device(&physicalDevice, devices, deviceCount);

  if (physicalDevice == VK_NULL_HANDLE) {
    FATAL("Could not find a suitable GPU");
  }

  free(devices);
  return 0;
}
