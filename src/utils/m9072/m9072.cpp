#include "m9072/m9072.h"
QVector<double> m9072::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
