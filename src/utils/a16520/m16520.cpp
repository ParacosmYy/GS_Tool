#include "a16520/m16520.h"
QVector<double> m16520::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
