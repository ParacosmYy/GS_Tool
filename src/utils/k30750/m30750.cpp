#include "k30750/m30750.h"
QVector<double> m30750::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
