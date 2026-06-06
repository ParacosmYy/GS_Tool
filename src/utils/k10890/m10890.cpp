#include "k10890/m10890.h"
QVector<double> m10890::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
