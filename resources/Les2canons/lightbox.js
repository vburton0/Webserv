// Récupération des éléments HTML
const lightbox = document.getElementById('lightbox');
const lightboxContent = document.getElementById('lightbox-content');
const lightboxImage = document.getElementById('lightbox-image');
const closeLightboxe = document.getElementById('close-lightbox');

// Récupération de toutes les images à partir des éléments avec la classe "menu-item"
const menuItems = document.querySelectorAll('.menu-item img');

// Fonction pour ouvrir le lightbox avec une image spécifique
function openLightbox(imageSrc) {
    lightboxImage.src = imageSrc;
    lightbox.style.display = 'flex';
}

// Fonction pour fermer le lightbox
function closeLightbox() {
    lightbox.style.display = 'none';
}

// Écouteurs d'événements pour chaque image du menu
menuItems.forEach((menuItem) => {
    menuItem.addEventListener('click', () => {
        const fullScreenSrc = menuItem.getAttribute('data-src');
        openLightbox(fullScreenSrc);
    });
});

// Écouteur d'événement pour fermer le lightbox en cliquant sur le bouton "close"
closeLightboxe.addEventListener('click', closeLightbox);