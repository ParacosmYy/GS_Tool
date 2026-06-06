#include "i36048/m36048.h"
QVector<double> m36048::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
