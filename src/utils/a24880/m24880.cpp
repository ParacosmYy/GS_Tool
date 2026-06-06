#include "a24880/m24880.h"
QVector<double> m24880::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
