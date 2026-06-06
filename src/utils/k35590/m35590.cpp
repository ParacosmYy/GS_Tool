#include "k35590/m35590.h"
QVector<double> m35590::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
