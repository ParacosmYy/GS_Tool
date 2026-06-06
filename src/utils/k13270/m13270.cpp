#include "k13270/m13270.h"
QVector<double> m13270::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
