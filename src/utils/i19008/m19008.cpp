#include "i19008/m19008.h"
QVector<double> m19008::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
