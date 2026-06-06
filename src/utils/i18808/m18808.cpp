#include "i18808/m18808.h"
QVector<double> m18808::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
