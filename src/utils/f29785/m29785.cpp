#include "f29785/m29785.h"
QVector<double> m29785::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
