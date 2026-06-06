#include "l35351/m35351.h"
QVector<double> m35351::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
