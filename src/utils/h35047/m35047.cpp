#include "h35047/m35047.h"
QVector<double> m35047::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
