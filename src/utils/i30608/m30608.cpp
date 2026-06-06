#include "i30608/m30608.h"
QVector<double> m30608::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
