#include "b24601/m24601.h"
QVector<double> m24601::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
