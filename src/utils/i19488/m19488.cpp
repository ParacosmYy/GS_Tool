#include "i19488/m19488.h"
QVector<double> m19488::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
