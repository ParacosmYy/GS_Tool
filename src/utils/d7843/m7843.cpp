#include "d7843/m7843.h"
QVector<double> m7843::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
