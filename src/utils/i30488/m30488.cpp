#include "i30488/m30488.h"
QVector<double> m30488::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
