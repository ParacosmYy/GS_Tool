#include "i26328/m26328.h"
QVector<double> m26328::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
