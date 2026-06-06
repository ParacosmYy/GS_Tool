#include "a29080/m29080.h"
QVector<double> m29080::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
