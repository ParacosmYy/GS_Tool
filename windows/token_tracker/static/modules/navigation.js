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
  const header = document.querySelector('.site-header');
  if (root.dataset.scrollContextReady === 'true') return;
  root.dataset.scrollContextReady = 'true';
  let frame = 0;

  /**
   * Recalculate the visual scroll context after asynchronous sections settle.
   * Dashboard charts, records, and disclosure panels can change document
   * height without producing a scroll event; observing the document body keeps
   * the progress trace honest without polling or coupling navigation to data.
   */
  const schedule = () => {
    if (!frame) frame = window.requestAnimationFrame(render);
  };

  const render = () => {
    const documentHeight = Math.max(document.documentElement.scrollHeight - window.innerHeight, 1);
    const progress = Math.min(100, Math.max(0, (window.scrollY / documentHeight) * 100));
    root.style.setProperty('--scroll-progress', `${progress.toFixed(2)}%`);
    root.style.setProperty('--scroll-progress-scale', (progress / 100).toFixed(4));
    header?.classList.toggle('is-scrolled', window.scrollY > 16);
    frame = 0;
  };

  window.addEventListener('scroll', schedule, { passive: true });
  window.addEventListener('resize', schedule, { passive: true });
  if ('ResizeObserver' in window) {
    const observer = new ResizeObserver(schedule);
    observer.observe(document.body);
  }
  render();
}

function sameDocumentSection(link) {
  const target = new URL(link.href, window.location.href);
  return target.pathname === window.location.pathname && target.hash;
}

function anchorTarget(link) {
  if (!sameDocumentSection(link)) return null;
  const targetUrl = new URL(link.href, window.location.href);
  const targetId = decodeURIComponent(targetUrl.hash.slice(1));
  const target = document.getElementById(targetId);
  return target ? { target, targetUrl } : null;
}

function targetScrollTop(target) {
  const marginTop = Number.parseFloat(getComputedStyle(target).scrollMarginTop) || 0;
  return Math.max(0, target.getBoundingClientRect().top + window.scrollY - marginTop);
}

/**
 * Replace distance-dependent native scrolling with bounded, cancellable motion.
 * The target remains a normal anchor for semantics; only the visual transition
 * is owned here so keyboard, touch, reduced-motion, and sharing behavior stay
 * aligned with the browser contract.
 *
 * @param {HTMLAnchorElement[]} anchors Same-document anchors on the page.
 * @param {HTMLAnchorElement[]} navigationLinks Section links used for state.
 */
function setupAnchorScroll(anchors, navigationLinks) {
  const root = document.documentElement;
  if (root.dataset.anchorScrollReady === 'true') return;
  const sectionAnchors = anchors.filter((anchor) => anchorTarget(anchor));
  if (!sectionAnchors.length) return;
  root.dataset.anchorScrollReady = 'true';

  let animationFrame = 0;
  let restoreScrollBehavior = null;

  const cancel = () => {
    if (animationFrame) window.cancelAnimationFrame(animationFrame);
    animationFrame = 0;
    if (restoreScrollBehavior) restoreScrollBehavior();
    restoreScrollBehavior = null;
  };

  const updateAddress = (targetUrl) => {
    window.history.pushState(null, '', `${targetUrl.pathname}${targetUrl.search}${targetUrl.hash}`);
  };

  const scrollToTarget = (anchor, event) => {
    const details = anchorTarget(anchor);
    if (!details || event.defaultPrevented || event.button !== 0
      || event.metaKey || event.ctrlKey || event.shiftKey || event.altKey) return;
    event.preventDefault();
    cancel();

    const target = details.target;
    const destination = targetScrollTop(target);
    const navigationLink = navigationLinks.find((link) => link.hash === details.targetUrl.hash);
    if (navigationLink) setActive(navigationLinks, navigationLink);
    updateAddress(details.targetUrl);

    if (window.matchMedia('(prefers-reduced-motion: reduce)').matches
      || Math.abs(destination - window.scrollY) < 1) {
      root.style.scrollBehavior = 'auto';
      window.scrollTo(0, destination);
      root.style.scrollBehavior = '';
      return;
    }

    const start = window.scrollY;
    const distance = destination - start;
    const duration = Math.min(760, Math.max(420, Math.abs(distance) * .28));
    const startedAt = performance.now();
    const previousBehavior = root.style.scrollBehavior;
    root.style.scrollBehavior = 'auto';
    restoreScrollBehavior = () => {
      root.style.scrollBehavior = previousBehavior;
    };

    const ease = (progress) => 1 - Math.pow(1 - progress, 3);
    const render = (now) => {
      const progress = Math.min(1, (now - startedAt) / duration);
      window.scrollTo(0, start + distance * ease(progress));
      if (progress < 1) {
        animationFrame = window.requestAnimationFrame(render);
        return;
      }
      animationFrame = 0;
      window.scrollTo(0, targetScrollTop(target));
      if (restoreScrollBehavior) restoreScrollBehavior();
      restoreScrollBehavior = null;
    };
    animationFrame = window.requestAnimationFrame(render);
  };

  sectionAnchors.forEach((anchor) => {
    anchor.addEventListener('click', (event) => scrollToTarget(anchor, event));
  });
  window.addEventListener('wheel', cancel, { passive: true });
  window.addEventListener('touchstart', cancel, { passive: true });
  window.addEventListener('pointerdown', cancel, { passive: true });
  window.addEventListener('keydown', cancel, { passive: true });
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
  setupScrollProgress();
  const nav = document.querySelector('.site-nav');
  const anchors = [...document.querySelectorAll('a[href]')];
  if (!nav || nav.dataset.navigationReady === 'true') {
    setupAnchorScroll(anchors, nav ? [...nav.querySelectorAll('a')] : []);
    return;
  }
  nav.dataset.navigationReady = 'true';
  const links = [...nav.querySelectorAll('a')];
  setupRouteContext(links);
  setupSectionContext(links);
  setupAnchorScroll(anchors, links);
}
