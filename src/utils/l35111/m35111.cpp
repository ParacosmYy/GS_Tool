#include "l35111/m35111.h"
QVector<double> m35111::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
