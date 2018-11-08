cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

file(COPY ${CMAKE_CURRENT_LIST_DIR}/features DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(BundleInstall ALL COMMAND bundle install --path vendor/bundle)
add_test(NAME DrafterIntegration COMMAND bundle exec cucumber)
set_tests_properties(DrafterIntegration PROPERTIES ENVIRONMENT "DRAFTER_BINARY_DIR=${Drafter_BINARY_DIR}")
