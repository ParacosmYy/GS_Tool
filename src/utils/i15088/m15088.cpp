#include "i15088/m15088.h"
QVector<double> m15088::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
