#include "d35503/m35503.h"
QVector<double> m35503::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
