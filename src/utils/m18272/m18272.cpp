#include "m18272/m18272.h"
QVector<double> m18272::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
