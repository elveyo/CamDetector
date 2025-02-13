using Microsoft.AspNetCore.Authorization;
using Microsoft.AspNetCore.Mvc;

namespace IoTPhotoTakerBackend.Controllers
{
    [ApiController]
    [Route("[controller]")]
    public class ImagesController : ControllerBase
    {
        [HttpPost]
        public async Task<IActionResult> UploadImage()
        {
            using (var stream = new MemoryStream())
            {
                await Request.Body.CopyToAsync(stream);
                stream.Position = 0;

                var uploadsPath = Path.Combine(Directory.GetCurrentDirectory(), "uploads");

                if (!Directory.Exists(uploadsPath))
                {
                    Directory.CreateDirectory(uploadsPath);
                }

                var filePath = Path.Combine(uploadsPath, $"{Guid.NewGuid()}.dib");

                await System.IO.File.WriteAllBytesAsync(filePath, stream.ToArray());

                return Ok("File uploaded successfully.");
            }
        }
        [HttpGet]

        public IActionResult GetImages()
        {
            var imagesPath = Path.Combine(Directory.GetCurrentDirectory(), "uploads");
            if (!Directory.Exists(imagesPath))
            {
                return NotFound("No images!");
            }
            var images = Directory.GetFiles(imagesPath)
                .Select(file => new
                {
                    Data = Convert.ToBase64String(System.IO.File.ReadAllBytes(file)),
                    CreatedAt = System.IO.File.GetCreationTime(file)// Dobijamo vreme kreiranja fajla

                });
            return Ok(images);
        }
    }
    }
