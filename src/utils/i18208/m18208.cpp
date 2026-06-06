#include "i18208/m18208.h"
QVector<double> m18208::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
