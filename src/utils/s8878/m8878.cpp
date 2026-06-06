#include "s8878/m8878.h"
QVector<double> m8878::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
