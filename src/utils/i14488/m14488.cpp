#include "i14488/m14488.h"
QVector<double> m14488::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
