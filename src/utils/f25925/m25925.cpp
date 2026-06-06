#include "f25925/m25925.h"
QVector<double> m25925::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
