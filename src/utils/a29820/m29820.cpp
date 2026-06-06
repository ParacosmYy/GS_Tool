#include "a29820/m29820.h"
QVector<double> m29820::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
