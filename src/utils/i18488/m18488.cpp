#include "i18488/m18488.h"
QVector<double> m18488::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
