#include "k35830/m35830.h"
QVector<double> m35830::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
