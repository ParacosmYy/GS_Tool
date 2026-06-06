#include "i18848/m18848.h"
QVector<double> m18848::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
