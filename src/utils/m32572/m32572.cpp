#include "m32572/m32572.h"
QVector<double> m32572::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
