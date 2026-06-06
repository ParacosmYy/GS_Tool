#include "i35008/m35008.h"
QVector<double> m35008::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
