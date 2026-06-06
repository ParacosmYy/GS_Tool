#include "d35603/m35603.h"
QVector<double> m35603::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
