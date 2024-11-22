import {Component, ViewChild, ElementRef, NgZone} from '@angular/core';
import {CommonModule} from '@angular/common';
import {HttpClient} from '@angular/common/http';
import {FormsModule} from '@angular/forms';
import {EmscriptenWasmComponent} from 'src/app/emscripten-wasm.component';

const getFileName = (filePath: string) => filePath.split("/").reverse()[0];

const allowedMimeTypes = ["image/bmp", "image/x-windows-bmp", "image/jpeg", "image/pjpeg", "image/png"];

const defaultImage = "assets/img/cube/angular.png";

const requestFullscreen =
  document.documentElement.requestFullscreen ;

@Component({
  templateUrl: "./cube.component.html",
  styleUrls: ["./cube.component.css"],
  imports: [CommonModule, FormsModule],
  standalone: true
})
export class CubeComponent extends EmscriptenWasmComponent {
  @ViewChild("canvas") canvas!: ElementRef;
  predefinedImages: string[];
  error!: string;
  fileUploadAccept: string;
  supportsFullscreen: boolean;

  constructor(private httpClient: HttpClient, private ngZone: NgZone) {
    super("CubeModule",  'assets/wasm/cube/cube.js', 'assets/wasm/cube/cube.wasm');

    this.supportsFullscreen = !!requestFullscreen;
    this.fileUploadAccept = allowedMimeTypes.join(",");
    this.predefinedImages = [defaultImage, "assets/img/cube/cat.png", "assets/img/cube/embroidery.png"];

    this.moduleDecorator = (mod) => {
      mod.arguments = [getFileName(defaultImage)];
      mod.preRun = [
        () => {
          mod.FS_createPreloadedFile!("/", getFileName(defaultImage), defaultImage, true);
        },
      ];
      mod.canvas = <HTMLCanvasElement>this.canvas.nativeElement;
      mod.printErr = (what: string) => {
        if (!what.startsWith("WARNING")) {
          this.ngZone.run(() => (this.error = what));
        }
      };
    };
  }

  toggleFullscreen() {
    if (requestFullscreen) {
      requestFullscreen.bind(this.canvas.nativeElement)();
    }
  }

  selectPredefinedImage(index: number) {
    //this.error = null;

    const imageUrl: string = this.predefinedImages[index];
    this.httpClient
      .get(imageUrl, { responseType: "arraybuffer" })
      .subscribe((imageBytes) => this.setTexture(getFileName(imageUrl), new Uint8Array(imageBytes)));
  }

  onFileUploaded(event: any) {
    const files = event.srcElement.file;
    if (!files.length) {
      return;
    }

    //this.error = null;

    const file = files[0];
    if (allowedMimeTypes.indexOf(file.type) < 0) {
      this.error = `Unsupported mime type ${file.type}`;
      return;
    }

    const fileName = file.name;

    const reader = new FileReader();
    reader.onload = () => {
      const inputArray = new Uint8Array(<ArrayBuffer>reader.result);
      this.setTexture(fileName, inputArray);
    };
    reader.readAsArrayBuffer(file);
  }

  private setTexture(fileName: string, inputArray: Uint8Array) {
    const isDefaultImage = fileName === getFileName(defaultImage);

    // Default image is always there
    if (!isDefaultImage) {
      this.module.FS_createDataFile!("/", fileName, inputArray, true);
    }

    this.module.ccall!("set_texture", "void", ["string"], [fileName]);

    // Delete the file afterwards to free memory
    if (!isDefaultImage) {
      this.module.FS_unlink!(fileName);
    }
  }
}
