#include "i18408/m18408.h"
QVector<double> m18408::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
