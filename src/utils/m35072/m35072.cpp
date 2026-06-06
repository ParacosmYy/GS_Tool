#include "m35072/m35072.h"
QVector<double> m35072::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
