/*
 * Author: AI Token Tracker Engineering Team
 * Maintainer: Project Owner
 * Purpose: Coordinate route context, section focus, and scroll progress.
 * Module: Web presentation / navigation contract
 */

function setActive(links, activeLink) {
  links.forEach((link) => {
    const active = link === activeLink;
    link.classList.toggle('is-active', active);
    if (active) link.setAttribute('aria-current', 'location');
    else link.removeAttribute('aria-current');
  });
}

function setupScrollProgress() {
  const root = document.documentElement;
  let frame = 0;

  const render = () => {
    const documentHeight = Math.max(document.documentElement.scrollHeight - window.innerHeight, 1);
    const progress = Math.min(100, Math.max(0, (window.scrollY / documentHeight) * 100));
    root.style.setProperty('--scroll-progress', `${progress.toFixed(2)}%`);
    frame = 0;
  };

  const schedule = () => {
    if (!frame) frame = window.requestAnimationFrame(render);
  };

  window.addEventListener('scroll', schedule, { passive: true });
  window.addEventListener('resize', schedule, { passive: true });
  render();
}

function sameDocumentSection(link) {
  const target = new URL(link.href, window.location.href);
  return target.pathname === window.location.pathname && target.hash;
}

function setupSectionContext(links) {
  const sectionLinks = links.filter((link) => sameDocumentSection(link));
  const sections = sectionLinks
    .map((link) => document.getElementById(new URL(link.href, window.location.href).hash.slice(1)))
    .filter(Boolean);
  if (!sectionLinks.length || !sections.length) return;

  const linkForSection = new Map(sectionLinks.map((link) => [
    new URL(link.href, window.location.href).hash.slice(1),
    link
  ]));
  const hashLink = sectionLinks.find((link) => link.hash === window.location.hash);
  setActive(sectionLinks, hashLink || sectionLinks[0]);

  if (!("IntersectionObserver" in window)) return;
  const observer = new IntersectionObserver((entries) => {
    const visible = entries
      .filter((entry) => entry.isIntersecting)
      .sort((left, right) => right.intersectionRatio - left.intersectionRatio)[0];
    if (visible) setActive(sectionLinks, linkForSection.get(visible.target.id));
  }, { rootMargin: '-18% 0px -54% 0px', threshold: [0, .2, .45, .8] });
  sections.forEach((section) => observer.observe(section));
  window.addEventListener('hashchange', () => {
    const next = sectionLinks.find((link) => link.hash === window.location.hash);
    if (next) setActive(sectionLinks, next);
  }, { passive: true });
}

function setupRouteContext(links) {
  const routeLink = links.find((link) => {
    const target = new URL(link.href, window.location.href);
    return target.pathname === window.location.pathname && !target.hash;
  });
  if (routeLink) setActive(links, routeLink);
}

export function setupNavigation() {
  const nav = document.querySelector('.site-nav');
  if (!nav || nav.dataset.navigationReady === 'true') return;
  nav.dataset.navigationReady = 'true';
  const links = [...nav.querySelectorAll('a')];
  setupRouteContext(links);
  setupSectionContext(links);
  setupScrollProgress();
}
