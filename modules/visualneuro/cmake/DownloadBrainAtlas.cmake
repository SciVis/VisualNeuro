
# Download the MNI Colin 27 brain atlas  to
# |download_dir|. 
# Visit http://www.bic.mni.mcgill.ca/ServicesAtlases/Colin27Highres

function(DownloadBrainAtlas download_dir)
  # Specify the binary distribution type and download directory.
  set(AVG_SCANS_DOWNLOAD_PATH "${download_dir}/average/mni_colin27_2008_nifti.zip")
  set(FUZZY_DOWNLOAD_PATH "${download_dir}/fuzzy/mni_colin27_2008_fuzzy_nifti.zip")

  # Download and/or extract the binary distribution if necessary.
    if(NOT EXISTS "${download_dir}/average" OR NOT EXISTS "${download_dir}/fuzzy")
      set(AVG_SCAN_DOWNLOAD_URL_ORIG "http://packages.bic.mni.mcgill.ca/mni-models/colin27/mni_colin27_2008_nifti.zip")
      # URL-encode the string
      string(REPLACE "+" "%2B" AVG_SCAN_DOWNLOAD_URL_ORIG ${AVG_SCAN_DOWNLOAD_URL_ORIG})

      # Download the binary distribution and verify the hash.
      message(STATUS "Downloading ${AVG_SCANS_DOWNLOAD_PATH}...")
      file(
        DOWNLOAD "${AVG_SCAN_DOWNLOAD_URL_ORIG}" "${AVG_SCANS_DOWNLOAD_PATH}"
        SHOW_PROGRESS
        )
        set(FUZZY_DOWNLOAD_URL_ORIG "http://packages.bic.mni.mcgill.ca/mni-models/colin27/mni_colin27_2008_fuzzy_nifti.zip")
      # URL-encode the string
      string(REPLACE "+" "%2B" FUZZY_DOWNLOAD_URL_ORIG ${FUZZY_DOWNLOAD_URL_ORIG})

      # Download the binary distribution and verify the hash.
      message(STATUS "Downloading ${FUZZY_DOWNLOAD_PATH}...")
      file(
        DOWNLOAD "${FUZZY_DOWNLOAD_URL_ORIG}" "${FUZZY_DOWNLOAD_PATH}"
        SHOW_PROGRESS
        )

            # Extract the binary distribution.
    message(STATUS "Extracting ${AVG_SCANS_DOWNLOAD_PATH}...")
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E tar xzf "${AVG_SCANS_DOWNLOAD_PATH}"
      WORKING_DIRECTORY "${download_dir}/average"
      )
    
    message(STATUS "Extracting ${FUZZY_DOWNLOAD_PATH}...")
    execute_process(
      COMMAND ${CMAKE_COMMAND} -E tar xzf "${FUZZY_DOWNLOAD_PATH}"
      WORKING_DIRECTORY "${download_dir}/fuzzy"
      )
    
    endif()
  file(REMOVE "${AVG_SCANS_DOWNLOAD_PATH}")
  file(REMOVE "${FUZZY_DOWNLOAD_PATH}")
endfunction()
