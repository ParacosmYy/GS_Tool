#include "f24565/m24565.h"
QVector<double> m24565::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
