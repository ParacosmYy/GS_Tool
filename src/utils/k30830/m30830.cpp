#include "k30830/m30830.h"
QVector<double> m30830::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
