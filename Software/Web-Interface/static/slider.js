const slider = document.querySelector("#slider");
const thumb = document.querySelector("#thumb");

slider.addEventListener("input", () => {
  const value = slider.value;
  thumb.style.left = `${value}%`;
});