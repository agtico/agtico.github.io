---
layout: report
title: "Fluidstack at $18B | Series B Evaluation"
date: "2026-09-02 12:00:00 +0000"
summary: "AGTI private-markets evaluation. Access restricted."
category: AGTI Research
pearl_report: true
report_css_version: 20260902c
sitemap: false
tags:
  - AGTI
  - Fluidstack
  - AI Infrastructure
  - Data Centers
  - Private Markets
---

<style>
.fs-gate{max-width:520px;margin:40px auto;border:1px solid rgba(255,45,32,0.5);background:rgba(10,14,14,0.96);padding:28px 26px}
.fs-gate .label{display:block;color:var(--muted);font:900 10px var(--mono);letter-spacing:.18em;text-transform:uppercase;margin-bottom:12px}
.fs-gate p{margin:0 0 16px;color:rgba(220,229,224,0.85);font-size:14px;line-height:1.5}
.fs-gate form{display:flex;gap:8px}
.fs-gate input{flex:1;background:#050707;border:1px solid var(--line-strong);color:var(--text);padding:10px 12px;font:14px var(--mono)}
.fs-gate button{background:rgba(255,45,32,0.12);border:1px solid var(--red-2,#ff5a42);color:var(--red-2,#ff5a42);padding:10px 16px;font:900 11px var(--mono);letter-spacing:.14em;text-transform:uppercase;cursor:pointer}
.fs-gate .status{margin-top:10px;min-height:18px;color:#ffb347;font:12px var(--mono)}
</style>

<div class="fs-gate" id="fs-gate">
  <span class="label">Restricted</span>
  <p>This evaluation is password protected. Enter the access password to decrypt it in your browser.</p>
  <form id="fs-gate-form" autocomplete="off">
    <input id="fs-gate-pw" type="password" placeholder="Password" aria-label="Password" />
    <button type="submit">Unlock</button>
  </form>
  <div class="status" id="fs-gate-status"></div>
</div>

<script>
(function(){
  var SRC="/assets/research/fluidstack/fluidstack-series-b-2026-09-02.enc.json";
  var gate=document.getElementById("fs-gate"), form=document.getElementById("fs-gate-form"),
      pwEl=document.getElementById("fs-gate-pw"), status=document.getElementById("fs-gate-status");
  var container=gate.parentNode;
  function b64(s){var bin=atob(s),u=new Uint8Array(bin.length);for(var i=0;i<bin.length;i++)u[i]=bin.charCodeAt(i);return u;}
  async function unlock(pw){
    status.textContent="Decrypting...";
    var res=await fetch(SRC,{cache:"force-cache"}); if(!res.ok) throw new Error("payload unavailable");
    var j=await res.json();
    var enc=new TextEncoder();
    var base=await crypto.subtle.importKey("raw",enc.encode(pw),"PBKDF2",false,["deriveKey"]);
    var key=await crypto.subtle.deriveKey({name:"PBKDF2",salt:b64(j.salt),iterations:j.iters,hash:"SHA-256"},base,{name:"AES-GCM",length:256},false,["decrypt"]);
    var pt=await crypto.subtle.decrypt({name:"AES-GCM",iv:b64(j.iv)},key,b64(j.ct));
    var html=new TextDecoder().decode(pt);
    container.innerHTML=html;
    try{sessionStorage.setItem("fs-gate-pw",pw);}catch(e){}
  }
  form.addEventListener("submit",function(ev){
    ev.preventDefault();
    unlock(pwEl.value).catch(function(){status.textContent="Incorrect password.";pwEl.value="";pwEl.focus();});
  });
  try{var saved=sessionStorage.getItem("fs-gate-pw"); if(saved){unlock(saved).catch(function(){});}}catch(e){}
})();
</script>
