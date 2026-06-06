#include "l18511/m18511.h"
QVector<double> m18511::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
