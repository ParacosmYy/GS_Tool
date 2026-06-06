#include "i35848/m35848.h"
QVector<double> m35848::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
