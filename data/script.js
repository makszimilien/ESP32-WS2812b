"use strict";

// Get elements
const inputsCard = document.getElementById("demos");
const blinkButton = document.getElementById("blink");
const snakeButton = document.getElementById("snake");
const diceButton = document.getElementById("dice");
const offButton = document.getElementById("off");
const colorPicker = document.getElementById("color");

// Callback for inputs
const handleInputs = function (event) {
  const xhr = new XMLHttpRequest();
  const params = new URLSearchParams();
  params.append(event.target.id, event.target.value);
  xhr.open("POST", "/input", true);
  xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
  xhr.send(params);
};

// Event listener for buttons
inputsCard.addEventListener("click", function (e) {
  e.preventDefault();
  console.log(e.target.id);
  handleInputs(e);
});

// Event listener for color picker
colorPicker.addEventListener("input", function (e) {
  // e.preventDefault();
  console.log(e.target.value);
  handleInputs(e);
});
