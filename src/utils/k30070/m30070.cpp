#include "k30070/m30070.h"
QVector<double> m30070::run(const QVector<double>&in) { m_s.c++; m_s.i+=in.size(); return in; }
