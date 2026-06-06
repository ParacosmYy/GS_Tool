#include "l35331/m35331.h"
QVector<double> m35331::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
