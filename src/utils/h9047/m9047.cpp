#include "h9047/m9047.h"
QVector<double> m9047::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
